#include "bastionx/vault/VaultService.h"
#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <ctime>

namespace bastionx {
namespace vault {

namespace fs = std::filesystem;

// === RAII wrapper for sqlite3* ===

class ScopedDb {
public:
    explicit ScopedDb(const std::string& path) : db_(nullptr) {
        int rc = sqlite3_open(path.c_str(), &db_);
        if (rc != SQLITE_OK) {
            std::string err = db_ ? sqlite3_errmsg(db_) : "unknown error";
            if (db_) sqlite3_close(db_);
            db_ = nullptr;
            throw std::runtime_error("Failed to open database: " + err);
        }
    }

    ~ScopedDb() {
        if (db_) sqlite3_close(db_);
    }

    ScopedDb(const ScopedDb&) = delete;
    ScopedDb& operator=(const ScopedDb&) = delete;

    sqlite3* get() const { return db_; }

private:
    sqlite3* db_;
};

// === RAII wrapper for sqlite3_stmt* ===

class ScopedStmt {
public:
    ScopedStmt(sqlite3* db, const std::string& sql) : stmt_(nullptr) {
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(
                "Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }
    }

    ~ScopedStmt() {
        if (stmt_) sqlite3_finalize(stmt_);
    }

    ScopedStmt(const ScopedStmt&) = delete;
    ScopedStmt& operator=(const ScopedStmt&) = delete;

    sqlite3_stmt* get() const { return stmt_; }

private:
    sqlite3_stmt* stmt_;
};

// Helper to execute a simple SQL statement
static void exec_sql(sqlite3* db, const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "unknown error";
        sqlite3_free(err_msg);
        throw std::runtime_error("SQL error: " + err);
    }
}

// === VaultService Implementation ===

VaultService::VaultService(const std::string& vault_path)
    : vault_path_(vault_path)
    , state_(fs::exists(vault_path) ? VaultState::kLocked : VaultState::kNoVault) {
}

VaultService::~VaultService() {
    wipe_keys();
}

bool VaultService::create(const std::string& password) {
    // Cannot create if vault already exists
    if (fs::exists(vault_path_)) {
        return false;
    }

    // Derive master key (generates random salt)
    auto derived = crypto::CryptoService::derive_master_key(password);

    // Open SQLite and create schema
    ScopedDb db(vault_path_);

    // Enable WAL mode for better performance
    exec_sql(db.get(), "PRAGMA journal_mode=WAL;");

    create_schema(db.get());

    // Store salt and KDF parameters
    salt_ = derived.salt;
    kdf_opslimit_ = crypto_pwhash_OPSLIMIT_MODERATE;
    kdf_memlimit_ = crypto_pwhash_MEMLIMIT_MODERATE;
    store_vault_meta(db.get());

    // Cache master key
    master_key_.emplace(std::move(derived.master_key));

    // Derive and cache verification subkey, store token
    verify_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_VERIFY));
    store_verify_token(db.get());

    // Derive and cache notes subkey
    notes_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_NOTES));

    state_ = VaultState::kUnlocked;
    return true;
}

bool VaultService::unlock(const std::string& password) {
    if (!fs::exists(vault_path_)) {
        return false;
    }

    if (state_ == VaultState::kUnlocked) {
        return true;  // Already unlocked
    }

    // Open DB and load metadata
    ScopedDb db(vault_path_);

    if (!load_vault_meta(db.get())) {
        return false;
    }

    // Derive master key using loaded salt
    auto derived = crypto::CryptoService::derive_master_key(password, salt_);

    // Cache master key temporarily for verification
    master_key_.emplace(std::move(derived.master_key));

    // Derive verification subkey
    verify_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_VERIFY));

    // Load and verify token
    std::array<uint8_t, crypto::CryptoService::NONCE_BYTES> nonce{};
    std::vector<uint8_t> ciphertext;

    if (!load_verify_token(db.get(), nonce, ciphertext)) {
        wipe_keys();
        return false;
    }

    // Attempt decryption
    crypto::CryptoService::EncryptedData token{std::move(ciphertext), nonce};
    auto plaintext = crypto::CryptoService::decrypt(token, *verify_subkey_, {});

    if (!plaintext.has_value()) {
        // Wrong password - MAC verification failed
        wipe_keys();
        state_ = VaultState::kLocked;
        return false;
    }

    // Verify marker matches
    if (plaintext->size() != VERIFY_MARKER_SIZE ||
        std::memcmp(plaintext->data(), VERIFY_MARKER, VERIFY_MARKER_SIZE) != 0) {
        wipe_keys();
        state_ = VaultState::kLocked;
        return false;
    }

    // Password verified - derive notes subkey
    notes_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_NOTES));

    state_ = VaultState::kUnlocked;
    return true;
}

void VaultService::lock() {
    wipe_keys();
    if (fs::exists(vault_path_)) {
        state_ = VaultState::kLocked;
    } else {
        state_ = VaultState::kNoVault;
    }
}

VaultState VaultService::state() const {
    return state_;
}

bool VaultService::is_unlocked() const {
    return state_ == VaultState::kUnlocked;
}

const crypto::SecureKey& VaultService::notes_subkey() const {
    if (state_ != VaultState::kUnlocked || !notes_subkey_.has_value()) {
        throw std::runtime_error("Vault is locked");
    }
    return *notes_subkey_;
}

const std::string& VaultService::vault_path() const {
    return vault_path_;
}

// === Private Helpers ===

void VaultService::wipe_keys() {
    // Resetting optionals triggers SecureBuffer destructor → sodium_memzero
    master_key_.reset();
    notes_subkey_.reset();
    verify_subkey_.reset();
}

void VaultService::create_schema(sqlite3* db) {
    exec_sql(db, R"(
        CREATE TABLE IF NOT EXISTS vault_meta (
            version      INTEGER NOT NULL DEFAULT 1,
            salt         BLOB NOT NULL,
            kdf_opslimit INTEGER NOT NULL,
            kdf_memlimit INTEGER NOT NULL,
            created_at   INTEGER NOT NULL
        );
    )");

    exec_sql(db, R"(
        CREATE TABLE IF NOT EXISTS vault_verify (
            nonce       BLOB NOT NULL,
            ciphertext  BLOB NOT NULL
        );
    )");

    exec_sql(db, R"(
        CREATE TABLE IF NOT EXISTS notes (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            nonce       BLOB NOT NULL,
            ciphertext  BLOB NOT NULL,
            created_at  INTEGER NOT NULL,
            updated_at  INTEGER NOT NULL
        );
    )");
}

void VaultService::store_vault_meta(sqlite3* db) {
    ScopedStmt stmt(db,
        "INSERT INTO vault_meta (version, salt, kdf_opslimit, kdf_memlimit, created_at) "
        "VALUES (?, ?, ?, ?, ?)");

    sqlite3_bind_int(stmt.get(), 1, 1);  // version
    sqlite3_bind_blob(stmt.get(), 2, salt_.data(), static_cast<int>(salt_.size()), SQLITE_STATIC);
    sqlite3_bind_int64(stmt.get(), 3, static_cast<sqlite3_int64>(kdf_opslimit_));
    sqlite3_bind_int64(stmt.get(), 4, static_cast<sqlite3_int64>(kdf_memlimit_));
    sqlite3_bind_int64(stmt.get(), 5, static_cast<sqlite3_int64>(std::time(nullptr)));

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(
            "Failed to store vault metadata: " + std::string(sqlite3_errmsg(db)));
    }
}

void VaultService::store_verify_token(sqlite3* db) {
    // Encrypt the known marker
    std::vector<uint8_t> marker(VERIFY_MARKER, VERIFY_MARKER + VERIFY_MARKER_SIZE);
    auto encrypted = crypto::CryptoService::encrypt(marker, *verify_subkey_, {});

    ScopedStmt stmt(db,
        "INSERT INTO vault_verify (nonce, ciphertext) VALUES (?, ?)");

    sqlite3_bind_blob(stmt.get(), 1, encrypted.nonce.data(),
                      static_cast<int>(encrypted.nonce.size()), SQLITE_STATIC);
    sqlite3_bind_blob(stmt.get(), 2, encrypted.ciphertext.data(),
                      static_cast<int>(encrypted.ciphertext.size()), SQLITE_STATIC);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(
            "Failed to store verify token: " + std::string(sqlite3_errmsg(db)));
    }
}

bool VaultService::load_vault_meta(sqlite3* db) {
    ScopedStmt stmt(db,
        "SELECT salt, kdf_opslimit, kdf_memlimit FROM vault_meta LIMIT 1");

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_ROW) {
        return false;  // No metadata found
    }

    // Load salt
    const void* salt_blob = sqlite3_column_blob(stmt.get(), 0);
    int salt_size = sqlite3_column_bytes(stmt.get(), 0);
    if (salt_size != static_cast<int>(crypto::CryptoService::SALT_BYTES) || salt_blob == nullptr) {
        return false;  // Invalid salt
    }
    std::memcpy(salt_.data(), salt_blob, crypto::CryptoService::SALT_BYTES);

    // Load KDF parameters
    kdf_opslimit_ = static_cast<uint64_t>(sqlite3_column_int64(stmt.get(), 1));
    kdf_memlimit_ = static_cast<uint64_t>(sqlite3_column_int64(stmt.get(), 2));

    return true;
}

bool VaultService::load_verify_token(
    sqlite3* db,
    std::array<uint8_t, crypto::CryptoService::NONCE_BYTES>& nonce,
    std::vector<uint8_t>& ciphertext
) {
    ScopedStmt stmt(db,
        "SELECT nonce, ciphertext FROM vault_verify LIMIT 1");

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_ROW) {
        return false;  // No verify token found
    }

    // Load nonce
    const void* nonce_blob = sqlite3_column_blob(stmt.get(), 0);
    int nonce_size = sqlite3_column_bytes(stmt.get(), 0);
    if (nonce_size != static_cast<int>(crypto::CryptoService::NONCE_BYTES) || nonce_blob == nullptr) {
        return false;
    }
    std::memcpy(nonce.data(), nonce_blob, crypto::CryptoService::NONCE_BYTES);

    // Load ciphertext
    const void* ct_blob = sqlite3_column_blob(stmt.get(), 1);
    int ct_size = sqlite3_column_bytes(stmt.get(), 1);
    if (ct_size <= 0 || ct_blob == nullptr) {
        return false;
    }
    ciphertext.assign(
        static_cast<const uint8_t*>(ct_blob),
        static_cast<const uint8_t*>(ct_blob) + ct_size);

    return true;
}

}  // namespace vault
}  // namespace bastionx
