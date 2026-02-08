#include "bastionx/storage/NotesRepository.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <cstring>
#include <chrono>

namespace bastionx {
namespace storage {

using json = nlohmann::json;

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

// === NotesRepository Implementation ===

NotesRepository::NotesRepository(const std::string& db_path)
    : db_(nullptr), db_path_(db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err = db_ ? sqlite3_errmsg(db_) : "unknown error";
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Failed to open database: " + err);
    }

    // Enable WAL mode
    exec_sql(db_, "PRAGMA journal_mode=WAL;");
}

NotesRepository::~NotesRepository() {
    close();
}

void NotesRepository::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool NotesRepository::is_open() const {
    return db_ != nullptr;
}

// === CRUD Operations ===

int64_t NotesRepository::create_note(const Note& note, const crypto::SecureKey& subkey) {
    int64_t now = current_timestamp();

    // Use a transaction: insert placeholder, get ID, encrypt with AAD, update
    exec_sql(db_, "BEGIN TRANSACTION;");

    try {
        // Insert placeholder row to get the auto-generated ID
        {
            ScopedStmt stmt(db_,
                "INSERT INTO notes (nonce, ciphertext, created_at, updated_at) "
                "VALUES (zeroblob(24), zeroblob(1), ?, ?)");
            sqlite3_bind_int64(stmt.get(), 1, now);
            sqlite3_bind_int64(stmt.get(), 2, now);

            int rc = sqlite3_step(stmt.get());
            if (rc != SQLITE_DONE) {
                throw std::runtime_error(
                    "Failed to insert placeholder: " + std::string(sqlite3_errmsg(db_)));
            }
        }

        int64_t note_id = sqlite3_last_insert_rowid(db_);

        // Serialize and encrypt with the real ID as AAD
        auto plaintext = serialize_note(note);
        auto aad = build_aad(note_id);
        auto encrypted = crypto::CryptoService::encrypt(plaintext, subkey, aad);

        // Update with real encrypted data
        {
            ScopedStmt stmt(db_,
                "UPDATE notes SET nonce = ?, ciphertext = ? WHERE id = ?");
            sqlite3_bind_blob(stmt.get(), 1, encrypted.nonce.data(),
                              static_cast<int>(encrypted.nonce.size()), SQLITE_STATIC);
            sqlite3_bind_blob(stmt.get(), 2, encrypted.ciphertext.data(),
                              static_cast<int>(encrypted.ciphertext.size()), SQLITE_STATIC);
            sqlite3_bind_int64(stmt.get(), 3, note_id);

            int rc = sqlite3_step(stmt.get());
            if (rc != SQLITE_DONE) {
                throw std::runtime_error(
                    "Failed to update note: " + std::string(sqlite3_errmsg(db_)));
            }
        }

        exec_sql(db_, "COMMIT;");
        return note_id;

    } catch (...) {
        exec_sql(db_, "ROLLBACK;");
        throw;
    }
}

std::optional<Note> NotesRepository::read_note(int64_t id, const crypto::SecureKey& subkey) {
    ScopedStmt stmt(db_,
        "SELECT id, nonce, ciphertext, created_at, updated_at FROM notes WHERE id = ?");
    sqlite3_bind_int64(stmt.get(), 1, id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_ROW) {
        return std::nullopt;  // Not found
    }

    // Extract nonce
    const void* nonce_blob = sqlite3_column_blob(stmt.get(), 1);
    int nonce_size = sqlite3_column_bytes(stmt.get(), 1);
    if (nonce_size != static_cast<int>(crypto::CryptoService::NONCE_BYTES) || nonce_blob == nullptr) {
        return std::nullopt;
    }

    std::array<uint8_t, crypto::CryptoService::NONCE_BYTES> nonce{};
    std::memcpy(nonce.data(), nonce_blob, crypto::CryptoService::NONCE_BYTES);

    // Extract ciphertext
    const void* ct_blob = sqlite3_column_blob(stmt.get(), 2);
    int ct_size = sqlite3_column_bytes(stmt.get(), 2);
    if (ct_size <= 0 || ct_blob == nullptr) {
        return std::nullopt;
    }

    std::vector<uint8_t> ciphertext(
        static_cast<const uint8_t*>(ct_blob),
        static_cast<const uint8_t*>(ct_blob) + ct_size);

    // Extract timestamps
    int64_t created_at = sqlite3_column_int64(stmt.get(), 3);
    int64_t updated_at = sqlite3_column_int64(stmt.get(), 4);

    // Decrypt
    auto aad = build_aad(id);
    crypto::CryptoService::EncryptedData encrypted{std::move(ciphertext), nonce};
    auto plaintext = crypto::CryptoService::decrypt(encrypted, subkey, aad);

    if (!plaintext.has_value()) {
        return std::nullopt;  // Decryption failed (tampered or wrong key)
    }

    // Deserialize
    auto note = deserialize_note(*plaintext);
    if (!note.has_value()) {
        return std::nullopt;  // JSON parse failed
    }

    // Populate DB-level fields
    note->id = id;
    note->created_at = created_at;
    note->updated_at = updated_at;

    return note;
}

std::vector<NoteSummary> NotesRepository::list_notes(const crypto::SecureKey& subkey) {
    std::vector<NoteSummary> summaries;

    ScopedStmt stmt(db_,
        "SELECT id, nonce, ciphertext, updated_at FROM notes ORDER BY updated_at DESC");

    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        int64_t id = sqlite3_column_int64(stmt.get(), 0);

        // Extract nonce
        const void* nonce_blob = sqlite3_column_blob(stmt.get(), 1);
        int nonce_size = sqlite3_column_bytes(stmt.get(), 1);
        if (nonce_size != static_cast<int>(crypto::CryptoService::NONCE_BYTES) || nonce_blob == nullptr) {
            continue;  // Skip corrupted row
        }

        std::array<uint8_t, crypto::CryptoService::NONCE_BYTES> nonce{};
        std::memcpy(nonce.data(), nonce_blob, crypto::CryptoService::NONCE_BYTES);

        // Extract ciphertext
        const void* ct_blob = sqlite3_column_blob(stmt.get(), 2);
        int ct_size = sqlite3_column_bytes(stmt.get(), 2);
        if (ct_size <= 0 || ct_blob == nullptr) {
            continue;  // Skip corrupted row
        }

        std::vector<uint8_t> ciphertext(
            static_cast<const uint8_t*>(ct_blob),
            static_cast<const uint8_t*>(ct_blob) + ct_size);

        int64_t updated_at = sqlite3_column_int64(stmt.get(), 3);

        // Decrypt
        auto aad = build_aad(id);
        crypto::CryptoService::EncryptedData encrypted{std::move(ciphertext), nonce};
        auto plaintext = crypto::CryptoService::decrypt(encrypted, subkey, aad);

        if (!plaintext.has_value()) {
            continue;  // Skip rows that fail to decrypt
        }

        // Deserialize and extract title only
        auto note = deserialize_note(*plaintext);
        if (!note.has_value()) {
            continue;
        }

        summaries.push_back(NoteSummary{id, std::move(note->title), updated_at});
    }

    return summaries;
}

bool NotesRepository::update_note(const Note& note, const crypto::SecureKey& subkey) {
    // Verify note exists
    {
        ScopedStmt check(db_, "SELECT id FROM notes WHERE id = ?");
        sqlite3_bind_int64(check.get(), 1, note.id);
        if (sqlite3_step(check.get()) != SQLITE_ROW) {
            return false;  // Note not found
        }
    }

    int64_t now = current_timestamp();

    // Serialize and encrypt with fresh nonce
    auto plaintext = serialize_note(note);
    auto aad = build_aad(note.id);
    auto encrypted = crypto::CryptoService::encrypt(plaintext, subkey, aad);

    // Update in DB
    ScopedStmt stmt(db_,
        "UPDATE notes SET nonce = ?, ciphertext = ?, updated_at = ? WHERE id = ?");
    sqlite3_bind_blob(stmt.get(), 1, encrypted.nonce.data(),
                      static_cast<int>(encrypted.nonce.size()), SQLITE_STATIC);
    sqlite3_bind_blob(stmt.get(), 2, encrypted.ciphertext.data(),
                      static_cast<int>(encrypted.ciphertext.size()), SQLITE_STATIC);
    sqlite3_bind_int64(stmt.get(), 3, now);
    sqlite3_bind_int64(stmt.get(), 4, note.id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(
            "Failed to update note: " + std::string(sqlite3_errmsg(db_)));
    }

    return true;
}

bool NotesRepository::delete_note(int64_t id) {
    ScopedStmt stmt(db_, "DELETE FROM notes WHERE id = ?");
    sqlite3_bind_int64(stmt.get(), 1, id);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(
            "Failed to delete note: " + std::string(sqlite3_errmsg(db_)));
    }

    return sqlite3_changes(db_) > 0;
}

// === Serialization Helpers ===

std::vector<uint8_t> NotesRepository::serialize_note(const Note& note) {
    json j;
    j["title"] = note.title;
    j["body"] = note.body;
    j["tags"] = note.tags;
    j["version"] = 1;

    std::string json_str = j.dump();
    return std::vector<uint8_t>(json_str.begin(), json_str.end());
}

std::optional<Note> NotesRepository::deserialize_note(const std::vector<uint8_t>& json_bytes) {
    std::string json_str(json_bytes.begin(), json_bytes.end());
    auto j = json::parse(json_str, nullptr, false);
    if (j.is_discarded()) {
        return std::nullopt;
    }

    Note note;
    note.title = j.value("title", "");
    note.body = j.value("body", "");
    note.tags = j.value("tags", std::vector<std::string>{});
    return note;
}

std::vector<uint8_t> NotesRepository::build_aad(int64_t note_id) {
    std::vector<uint8_t> aad(4);
    uint32_t id32 = static_cast<uint32_t>(note_id);
    std::memcpy(aad.data(), &id32, 4);  // Little-endian on x64
    return aad;
}

int64_t NotesRepository::current_timestamp() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()).count();
}

}  // namespace storage
}  // namespace bastionx
