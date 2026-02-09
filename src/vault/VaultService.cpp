#include "bastionx/vault/VaultService.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <ctime>

namespace bastionx {
namespace vault {

namespace fs = std::filesystem;

// Helper to execute a simple SQL statement (forward declaration for ScopedDb)
static void exec_sql(sqlite3* db, const std::string& sql);

// === RAII wrapper for sqlite3* ===

class ScopedDb {
public:
    explicit ScopedDb(const std::string& path, const crypto::SecureKey* db_key = nullptr) : db_(nullptr) {
        int rc = sqlite3_open(path.c_str(), &db_);
        if (rc != SQLITE_OK) {
            std::string err = db_ ? sqlite3_errmsg(db_) : "unknown error";
            if (db_) sqlite3_close(db_);
            db_ = nullptr;
            throw std::runtime_error("Failed to open database: " + err);
        }
        if (db_key) {
            rc = sqlite3_key(db_, db_key->data(), static_cast<int>(db_key->size()));
            if (rc != SQLITE_OK) {
                std::string err = sqlite3_errmsg(db_);
                sqlite3_close(db_);
                db_ = nullptr;
                throw std::runtime_error("Failed to set encryption key: " + err);
            }
            exec_sql(db_, "PRAGMA cipher_memory_security = ON;");
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

    // Write salt sidecar file (must happen before DB open)
    write_salt_file(derived.salt);

    // Derive database encryption subkey
    auto db_key = crypto::CryptoService::derive_subkey(
        derived.master_key, crypto::CryptoService::SUBKEY_DATABASE);

    // Open SQLite with encryption — DB is encrypted from birth
    ScopedDb db(vault_path_, &db_key);

    // Enable WAL mode (after keying)
    exec_sql(db.get(), "PRAGMA journal_mode=WAL;");

    create_schema(db.get());

    // Store salt and KDF parameters
    salt_ = derived.salt;
    kdf_opslimit_ = crypto_pwhash_OPSLIMIT_MODERATE;
    kdf_memlimit_ = crypto_pwhash_MEMLIMIT_MODERATE;
    store_vault_meta(db.get());

    // Cache master key
    master_key_.emplace(std::move(derived.master_key));

    // Cache database subkey
    db_subkey_.emplace(std::move(db_key));

    // Derive and cache verification subkey, store token
    verify_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_VERIFY));
    store_verify_token(db.get());

    // Derive and cache notes subkey
    notes_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_NOTES));

    // Derive and cache settings subkey
    settings_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_SETTINGS));

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

    // Step 1: Read salt from sidecar file (must happen before DB open)
    std::array<uint8_t, crypto::CryptoService::SALT_BYTES> file_salt{};
    bool has_salt_file = read_salt_file(file_salt);

    if (!has_salt_file) {
        // No salt sidecar — attempt migration from unencrypted (pre-Phase 5) vault
        return migrate_and_unlock(password);
    }
    salt_ = file_salt;

    // Step 2: Derive master key and database subkey using sidecar salt
    auto derived = crypto::CryptoService::derive_master_key(password, salt_);

    auto db_key = crypto::CryptoService::derive_subkey(
        derived.master_key, crypto::CryptoService::SUBKEY_DATABASE);

    // Step 3: Open encrypted DB and validate key
    // Wrong password → wrong db_key → SQLCipher throws on first query
    std::unique_ptr<ScopedDb> db_ptr;
    try {
        db_ptr = std::make_unique<ScopedDb>(vault_path_, &db_key);
        if (!load_vault_meta(db_ptr->get())) {
            state_ = VaultState::kLocked;
            return false;
        }
    } catch (const std::runtime_error&) {
        // Wrong encryption key — "file is not a database"
        state_ = VaultState::kLocked;
        return false;
    }
    auto& db = *db_ptr;

    // Cache master key temporarily for verification
    master_key_.emplace(std::move(derived.master_key));
    db_subkey_.emplace(std::move(db_key));

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

    // Derive and cache settings subkey
    settings_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_SETTINGS));

    // Migrate schema for pre-Phase 4 vaults
    migrate_schema(db.get());

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

const crypto::SecureKey& VaultService::settings_subkey() const {
    if (state_ != VaultState::kUnlocked || !settings_subkey_.has_value()) {
        throw std::runtime_error("Vault is locked");
    }
    return *settings_subkey_;
}

const crypto::SecureKey& VaultService::db_subkey() const {
    if (state_ != VaultState::kUnlocked || !db_subkey_.has_value()) {
        throw std::runtime_error("Vault is locked");
    }
    return *db_subkey_;
}

void VaultService::save_settings(const std::string& json_str) {
    if (state_ != VaultState::kUnlocked) {
        throw std::runtime_error("Vault is locked");
    }

    std::vector<uint8_t> plaintext(json_str.begin(), json_str.end());
    auto encrypted = crypto::CryptoService::encrypt(plaintext, *settings_subkey_, {});

    ScopedDb db(vault_path_, &*db_subkey_);

    // Ensure vault_settings table exists (migration for pre-Phase 4 vaults)
    migrate_schema(db.get());

    // Delete existing settings row, then insert new one
    exec_sql(db.get(), "DELETE FROM vault_settings;");

    ScopedStmt stmt(db.get(),
        "INSERT INTO vault_settings (nonce, ciphertext) VALUES (?, ?)");

    sqlite3_bind_blob(stmt.get(), 1, encrypted.nonce.data(),
                      static_cast<int>(encrypted.nonce.size()), SQLITE_STATIC);
    sqlite3_bind_blob(stmt.get(), 2, encrypted.ciphertext.data(),
                      static_cast<int>(encrypted.ciphertext.size()), SQLITE_STATIC);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(
            "Failed to save settings: " + std::string(sqlite3_errmsg(db.get())));
    }
}

std::string VaultService::load_settings() {
    if (state_ != VaultState::kUnlocked) {
        throw std::runtime_error("Vault is locked");
    }

    ScopedDb db(vault_path_, &*db_subkey_);

    // Ensure vault_settings table exists
    migrate_schema(db.get());

    ScopedStmt stmt(db.get(),
        "SELECT nonce, ciphertext FROM vault_settings LIMIT 1");

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_ROW) {
        return "";  // No settings stored yet
    }

    // Load nonce
    const void* nonce_blob = sqlite3_column_blob(stmt.get(), 0);
    int nonce_size = sqlite3_column_bytes(stmt.get(), 0);
    if (nonce_size != static_cast<int>(crypto::CryptoService::NONCE_BYTES) || nonce_blob == nullptr) {
        return "";
    }
    std::array<uint8_t, crypto::CryptoService::NONCE_BYTES> nonce{};
    std::memcpy(nonce.data(), nonce_blob, crypto::CryptoService::NONCE_BYTES);

    // Load ciphertext
    const void* ct_blob = sqlite3_column_blob(stmt.get(), 1);
    int ct_size = sqlite3_column_bytes(stmt.get(), 1);
    if (ct_size <= 0 || ct_blob == nullptr) {
        return "";
    }
    std::vector<uint8_t> ciphertext(
        static_cast<const uint8_t*>(ct_blob),
        static_cast<const uint8_t*>(ct_blob) + ct_size);

    // Decrypt
    crypto::CryptoService::EncryptedData enc{std::move(ciphertext), nonce};
    auto plaintext = crypto::CryptoService::decrypt(enc, *settings_subkey_, {});

    if (!plaintext.has_value()) {
        return "";  // Decryption failed (shouldn't happen with correct key)
    }

    return std::string(plaintext->begin(), plaintext->end());
}

bool VaultService::change_password(const std::string& current_password,
                                   const std::string& new_password) {
    if (state_ != VaultState::kUnlocked) {
        throw std::runtime_error("Vault is locked");
    }

    // Step 1: Verify current password by re-deriving master key
    auto current_derived = crypto::CryptoService::derive_master_key(current_password, salt_);
    auto current_verify = crypto::CryptoService::derive_subkey(
        current_derived.master_key, crypto::CryptoService::SUBKEY_VERIFY);

    // Check that the re-derived verify subkey matches by trying to decrypt the token
    ScopedDb db(vault_path_, &*db_subkey_);

    std::array<uint8_t, crypto::CryptoService::NONCE_BYTES> verify_nonce{};
    std::vector<uint8_t> verify_ct;
    if (!load_verify_token(db.get(), verify_nonce, verify_ct)) {
        return false;
    }

    crypto::CryptoService::EncryptedData verify_enc{std::move(verify_ct), verify_nonce};
    auto verify_plain = crypto::CryptoService::decrypt(verify_enc, current_verify, {});
    if (!verify_plain.has_value() ||
        verify_plain->size() != VERIFY_MARKER_SIZE ||
        std::memcmp(verify_plain->data(), VERIFY_MARKER, VERIFY_MARKER_SIZE) != 0) {
        return false;  // Current password is wrong
    }

    // Step 2: Derive new master key (new random salt)
    auto new_derived = crypto::CryptoService::derive_master_key(new_password);

    // Step 3: Derive all new subkeys
    auto new_notes_subkey = crypto::CryptoService::derive_subkey(
        new_derived.master_key, crypto::CryptoService::SUBKEY_NOTES);
    auto new_verify_subkey = crypto::CryptoService::derive_subkey(
        new_derived.master_key, crypto::CryptoService::SUBKEY_VERIFY);
    auto new_settings_subkey = crypto::CryptoService::derive_subkey(
        new_derived.master_key, crypto::CryptoService::SUBKEY_SETTINGS);
    auto new_db_subkey = crypto::CryptoService::derive_subkey(
        new_derived.master_key, crypto::CryptoService::SUBKEY_DATABASE);

    // Step 4: BEGIN EXCLUSIVE TRANSACTION
    exec_sql(db.get(), "BEGIN EXCLUSIVE TRANSACTION;");

    try {
        // Step 5: Re-encrypt all notes
        {
            ScopedStmt select_stmt(db.get(),
                "SELECT id, nonce, ciphertext FROM notes");

            // Collect all notes first (can't UPDATE while iterating SELECT)
            struct NoteRow {
                int64_t id;
                std::array<uint8_t, crypto::CryptoService::NONCE_BYTES> nonce;
                std::vector<uint8_t> ciphertext;
            };
            std::vector<NoteRow> rows;

            while (sqlite3_step(select_stmt.get()) == SQLITE_ROW) {
                NoteRow row;
                row.id = sqlite3_column_int64(select_stmt.get(), 0);

                const void* nonce_blob = sqlite3_column_blob(select_stmt.get(), 1);
                int nonce_size = sqlite3_column_bytes(select_stmt.get(), 1);
                if (nonce_size != static_cast<int>(crypto::CryptoService::NONCE_BYTES) || !nonce_blob) {
                    throw std::runtime_error("Corrupted note nonce during password change");
                }
                std::memcpy(row.nonce.data(), nonce_blob, crypto::CryptoService::NONCE_BYTES);

                const void* ct_blob = sqlite3_column_blob(select_stmt.get(), 2);
                int ct_size = sqlite3_column_bytes(select_stmt.get(), 2);
                if (ct_size <= 0 || !ct_blob) {
                    throw std::runtime_error("Corrupted note ciphertext during password change");
                }
                row.ciphertext.assign(
                    static_cast<const uint8_t*>(ct_blob),
                    static_cast<const uint8_t*>(ct_blob) + ct_size);

                rows.push_back(std::move(row));
            }

            // Decrypt with old key, re-encrypt with new key
            for (const auto& row : rows) {
                // Build AAD (note_id as 4-byte LE)
                std::vector<uint8_t> aad(4);
                uint32_t id32 = static_cast<uint32_t>(row.id);
                std::memcpy(aad.data(), &id32, 4);

                // Decrypt with old notes subkey
                crypto::CryptoService::EncryptedData old_enc{row.ciphertext, row.nonce};
                auto plaintext = crypto::CryptoService::decrypt(old_enc, *notes_subkey_, aad);
                if (!plaintext.has_value()) {
                    throw std::runtime_error("Failed to decrypt note " + std::to_string(row.id) +
                                             " during password change");
                }

                // Re-encrypt with new notes subkey (same AAD)
                auto new_enc = crypto::CryptoService::encrypt(*plaintext, new_notes_subkey, aad);

                // Update row
                ScopedStmt update_stmt(db.get(),
                    "UPDATE notes SET nonce = ?, ciphertext = ? WHERE id = ?");
                sqlite3_bind_blob(update_stmt.get(), 1, new_enc.nonce.data(),
                                  static_cast<int>(new_enc.nonce.size()), SQLITE_STATIC);
                sqlite3_bind_blob(update_stmt.get(), 2, new_enc.ciphertext.data(),
                                  static_cast<int>(new_enc.ciphertext.size()), SQLITE_STATIC);
                sqlite3_bind_int64(update_stmt.get(), 3, row.id);

                int rc = sqlite3_step(update_stmt.get());
                if (rc != SQLITE_DONE) {
                    throw std::runtime_error("Failed to re-encrypt note " + std::to_string(row.id));
                }
            }
        }

        // Step 6: Re-encrypt verify token
        {
            exec_sql(db.get(), "DELETE FROM vault_verify;");

            std::vector<uint8_t> marker(VERIFY_MARKER, VERIFY_MARKER + VERIFY_MARKER_SIZE);
            auto new_enc = crypto::CryptoService::encrypt(marker, new_verify_subkey, {});

            ScopedStmt stmt(db.get(),
                "INSERT INTO vault_verify (nonce, ciphertext) VALUES (?, ?)");
            sqlite3_bind_blob(stmt.get(), 1, new_enc.nonce.data(),
                              static_cast<int>(new_enc.nonce.size()), SQLITE_STATIC);
            sqlite3_bind_blob(stmt.get(), 2, new_enc.ciphertext.data(),
                              static_cast<int>(new_enc.ciphertext.size()), SQLITE_STATIC);

            int rc = sqlite3_step(stmt.get());
            if (rc != SQLITE_DONE) {
                throw std::runtime_error("Failed to re-encrypt verify token");
            }
        }

        // Step 7: Re-encrypt settings (if any exist)
        {
            migrate_schema(db.get());

            ScopedStmt select_stmt(db.get(),
                "SELECT nonce, ciphertext FROM vault_settings LIMIT 1");

            if (sqlite3_step(select_stmt.get()) == SQLITE_ROW) {
                const void* nonce_blob = sqlite3_column_blob(select_stmt.get(), 0);
                int nonce_size = sqlite3_column_bytes(select_stmt.get(), 0);
                const void* ct_blob = sqlite3_column_blob(select_stmt.get(), 1);
                int ct_size = sqlite3_column_bytes(select_stmt.get(), 1);

                if (nonce_size == static_cast<int>(crypto::CryptoService::NONCE_BYTES) &&
                    nonce_blob && ct_size > 0 && ct_blob) {

                    std::array<uint8_t, crypto::CryptoService::NONCE_BYTES> old_nonce{};
                    std::memcpy(old_nonce.data(), nonce_blob, crypto::CryptoService::NONCE_BYTES);

                    std::vector<uint8_t> old_ct(
                        static_cast<const uint8_t*>(ct_blob),
                        static_cast<const uint8_t*>(ct_blob) + ct_size);

                    crypto::CryptoService::EncryptedData old_enc{std::move(old_ct), old_nonce};
                    auto plaintext = crypto::CryptoService::decrypt(old_enc, *settings_subkey_, {});

                    if (plaintext.has_value()) {
                        exec_sql(db.get(), "DELETE FROM vault_settings;");

                        auto new_enc = crypto::CryptoService::encrypt(*plaintext, new_settings_subkey, {});

                        ScopedStmt ins(db.get(),
                            "INSERT INTO vault_settings (nonce, ciphertext) VALUES (?, ?)");
                        sqlite3_bind_blob(ins.get(), 1, new_enc.nonce.data(),
                                          static_cast<int>(new_enc.nonce.size()), SQLITE_STATIC);
                        sqlite3_bind_blob(ins.get(), 2, new_enc.ciphertext.data(),
                                          static_cast<int>(new_enc.ciphertext.size()), SQLITE_STATIC);

                        int rc = sqlite3_step(ins.get());
                        if (rc != SQLITE_DONE) {
                            throw std::runtime_error("Failed to re-encrypt settings");
                        }
                    }
                }
            }
        }

        // Step 8: Update vault_meta with new salt
        {
            exec_sql(db.get(), "DELETE FROM vault_meta;");

            ScopedStmt stmt(db.get(),
                "INSERT INTO vault_meta (version, salt, kdf_opslimit, kdf_memlimit, created_at) "
                "VALUES (?, ?, ?, ?, ?)");

            sqlite3_bind_int(stmt.get(), 1, 1);
            sqlite3_bind_blob(stmt.get(), 2, new_derived.salt.data(),
                              static_cast<int>(new_derived.salt.size()), SQLITE_STATIC);
            sqlite3_bind_int64(stmt.get(), 3,
                               static_cast<sqlite3_int64>(crypto_pwhash_OPSLIMIT_MODERATE));
            sqlite3_bind_int64(stmt.get(), 4,
                               static_cast<sqlite3_int64>(crypto_pwhash_MEMLIMIT_MODERATE));
            sqlite3_bind_int64(stmt.get(), 5,
                               static_cast<sqlite3_int64>(std::time(nullptr)));

            int rc = sqlite3_step(stmt.get());
            if (rc != SQLITE_DONE) {
                throw std::runtime_error("Failed to update vault metadata");
            }
        }

        // Step 9: COMMIT
        exec_sql(db.get(), "COMMIT;");

    } catch (...) {
        // ROLLBACK - old password remains valid
        exec_sql(db.get(), "ROLLBACK;");
        throw;
    }

    // Step 10: Re-key the database file with the new encryption key
    int rekey_rc = sqlite3_rekey(db.get(), new_db_subkey.data(),
                                 static_cast<int>(new_db_subkey.size()));
    if (rekey_rc != SQLITE_OK) {
        throw std::runtime_error(
            "Failed to re-key database: " + std::string(sqlite3_errmsg(db.get())));
    }

    // Step 11: Update salt sidecar file
    write_salt_file(new_derived.salt);

    // Step 12: Update in-memory keys only after everything succeeds
    salt_ = new_derived.salt;
    kdf_opslimit_ = crypto_pwhash_OPSLIMIT_MODERATE;
    kdf_memlimit_ = crypto_pwhash_MEMLIMIT_MODERATE;

    master_key_.emplace(std::move(new_derived.master_key));
    notes_subkey_.emplace(std::move(new_notes_subkey));
    verify_subkey_.emplace(std::move(new_verify_subkey));
    settings_subkey_.emplace(std::move(new_settings_subkey));
    db_subkey_.emplace(std::move(new_db_subkey));

    return true;
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
    settings_subkey_.reset();
    db_subkey_.reset();
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

    exec_sql(db, R"(
        CREATE TABLE IF NOT EXISTS vault_settings (
            nonce      BLOB NOT NULL,
            ciphertext BLOB NOT NULL
        );
    )");
}

void VaultService::migrate_schema(sqlite3* db) {
    // Add vault_settings table if it doesn't exist (for pre-Phase 4 vaults)
    exec_sql(db, R"(
        CREATE TABLE IF NOT EXISTS vault_settings (
            nonce      BLOB NOT NULL,
            ciphertext BLOB NOT NULL
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

// === Salt Sidecar File Helpers ===

std::string VaultService::salt_path(const std::string& vault_path) {
    auto p = fs::path(vault_path);
    return (p.parent_path() / (p.stem().string() + ".salt")).string();
}

void VaultService::write_salt_file(const std::array<uint8_t, crypto::CryptoService::SALT_BYTES>& salt) {
    std::ofstream f(salt_path(vault_path_), std::ios::binary | std::ios::trunc);
    if (!f) {
        throw std::runtime_error("Failed to write salt sidecar file");
    }
    f.write(reinterpret_cast<const char*>(salt.data()), salt.size());
}

bool VaultService::read_salt_file(std::array<uint8_t, crypto::CryptoService::SALT_BYTES>& salt) {
    std::ifstream f(salt_path(vault_path_), std::ios::binary);
    if (!f) return false;
    f.read(reinterpret_cast<char*>(salt.data()), salt.size());
    return f.gcount() == static_cast<std::streamsize>(salt.size());
}

// === Migration from Unencrypted (pre-Phase 5) Vaults ===

bool VaultService::migrate_and_unlock(const std::string& password) {
    std::optional<crypto::SecureKey> db_key;
    auto encrypted_path = vault_path_ + ".encrypted";
    auto backup_path = vault_path_ + ".bak";

    // Phase 1: Open plaintext DB, verify password, export encrypted copy
    // Nested scope ensures ScopedDb closes before file swap operations
    {
        ScopedDb plaintext_db(vault_path_);

        if (!load_vault_meta(plaintext_db.get())) {
            state_ = VaultState::kLocked;
            return false;
        }

        // Derive master key using salt from the plaintext vault_meta
        auto derived = crypto::CryptoService::derive_master_key(password, salt_);
        master_key_.emplace(std::move(derived.master_key));

        // Verify password via the verify token
        verify_subkey_.emplace(
            crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_VERIFY));

        std::array<uint8_t, crypto::CryptoService::NONCE_BYTES> nonce{};
        std::vector<uint8_t> ciphertext;
        if (!load_verify_token(plaintext_db.get(), nonce, ciphertext)) {
            wipe_keys();
            return false;
        }

        crypto::CryptoService::EncryptedData token{std::move(ciphertext), nonce};
        auto pt = crypto::CryptoService::decrypt(token, *verify_subkey_, {});

        if (!pt.has_value() ||
            pt->size() != VERIFY_MARKER_SIZE ||
            std::memcmp(pt->data(), VERIFY_MARKER, VERIFY_MARKER_SIZE) != 0) {
            wipe_keys();
            state_ = VaultState::kLocked;
            return false;
        }

        // Derive database encryption subkey
        db_key.emplace(crypto::CryptoService::derive_subkey(
            *master_key_, crypto::CryptoService::SUBKEY_DATABASE));

        // Build hex key string for ATTACH KEY
        std::string hex_key = "x'";
        for (size_t i = 0; i < db_key->size(); i++) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", (*db_key)[i]);
            hex_key += hex;
        }
        hex_key += "'";

        // Remove any pre-existing encrypted file from a previous failed migration
        std::error_code ec;
        fs::remove(encrypted_path, ec);

        // Export all data to encrypted copy via sqlcipher_export
        exec_sql(plaintext_db.get(),
            "ATTACH DATABASE '" + encrypted_path + "' AS encrypted KEY " + hex_key + ";");
        exec_sql(plaintext_db.get(), "SELECT sqlcipher_export('encrypted');");
        exec_sql(plaintext_db.get(), "DETACH DATABASE encrypted;");

    } // plaintext_db closes here — file handle released on Windows

    // Phase 2: Swap files (plaintext → backup, encrypted → vault.db)
    std::error_code ec;

    // Remove stale WAL/SHM files from the plaintext DB
    fs::remove(vault_path_ + "-wal", ec);
    fs::remove(vault_path_ + "-shm", ec);

    fs::rename(vault_path_, backup_path, ec);
    if (ec) {
        fs::remove(encrypted_path, ec);
        wipe_keys();
        return false;
    }

    fs::rename(encrypted_path, vault_path_, ec);
    if (ec) {
        // Restore backup
        fs::rename(backup_path, vault_path_, ec);
        wipe_keys();
        return false;
    }

    // Phase 3: Write salt sidecar file
    write_salt_file(salt_);

    // Phase 4: Finish unlock — derive remaining subkeys
    db_subkey_.emplace(std::move(*db_key));
    notes_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_NOTES));
    settings_subkey_.emplace(
        crypto::CryptoService::derive_subkey(*master_key_, crypto::CryptoService::SUBKEY_SETTINGS));

    // Verify the encrypted DB opens correctly and migrate schema
    {
        ScopedDb enc_db(vault_path_, &*db_subkey_);
        migrate_schema(enc_db.get());
    }

    // Clean up plaintext backup
    fs::remove(backup_path, ec);

    state_ = VaultState::kUnlocked;
    return true;
}

}  // namespace vault
}  // namespace bastionx
