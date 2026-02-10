#ifndef BASTIONX_STORAGE_NOTESREPOSITORY_H
#define BASTIONX_STORAGE_NOTESREPOSITORY_H

#include "bastionx/crypto/CryptoService.h"
#include "bastionx/crypto/SecureMemory.h"
#include <sqlcipher/sqlite3.h>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace bastionx {
namespace storage {

/**
 * @brief Decrypted note representation (in memory only, never persisted as plaintext)
 */
struct Note {
    int64_t id = 0;                      ///< DB primary key (0 = unsaved)
    std::string title;
    std::string body;
    std::vector<std::string> tags;
    int64_t created_at = 0;              ///< UNIX timestamp
    int64_t updated_at = 0;              ///< UNIX timestamp
};

/**
 * @brief Summary for sidebar listing (avoids holding full body in memory)
 */
struct NoteSummary {
    int64_t id = 0;
    std::string title;                   ///< Decrypted title only
    std::string preview;                 ///< First ~80 chars of body
    std::vector<std::string> tags;       ///< Tags for sidebar display
    int64_t updated_at = 0;
};

/**
 * @brief Encrypted CRUD operations for notes backed by SQLite
 *
 * NotesRepository owns a persistent SQLite connection and provides
 * create/read/update/delete operations on encrypted notes.
 *
 * All note payloads are serialized to JSON (via nlohmann-json), encrypted
 * using CryptoService, and stored as BLOB columns in SQLite.
 *
 * The subkey is passed per-call rather than stored, keeping key material
 * ownership explicit and confined to VaultService.
 */
class NotesRepository {
public:
    /**
     * @brief Open an existing vault database for note operations
     * @param db_path Path to the SQLite vault database (must already have schema)
     * @param db_key Optional SQLCipher encryption key (nullptr for unencrypted)
     * @throws std::runtime_error if database cannot be opened
     */
    explicit NotesRepository(const std::string& db_path,
                             const crypto::SecureKey* db_key = nullptr);
    ~NotesRepository();

    // Non-copyable (owns sqlite3* handle)
    NotesRepository(const NotesRepository&) = delete;
    NotesRepository& operator=(const NotesRepository&) = delete;

    // === CRUD Operations ===

    /**
     * @brief Create a new encrypted note
     * @param note Note data (id field is ignored, assigned by DB)
     * @param subkey Notes subkey from VaultService
     * @return Assigned note ID (> 0)
     * @throws std::runtime_error on SQLite or encryption errors
     */
    int64_t create_note(const Note& note, const crypto::SecureKey& subkey);

    /**
     * @brief Read and decrypt a note by ID
     * @param id Note ID
     * @param subkey Notes subkey from VaultService
     * @return Decrypted Note, or nullopt if not found or decryption fails
     */
    std::optional<Note> read_note(int64_t id, const crypto::SecureKey& subkey);

    /**
     * @brief List all notes (decrypted titles for sidebar)
     * @param subkey Notes subkey from VaultService
     * @return Vector of NoteSummary sorted by updated_at DESC
     *
     * @note Rows that fail to decrypt are skipped (not included in results)
     */
    std::vector<NoteSummary> list_notes(const crypto::SecureKey& subkey);

    /**
     * @brief Update an existing note (re-encrypts with fresh nonce)
     * @param note Note with id set and updated fields
     * @param subkey Notes subkey from VaultService
     * @return true if note was found and updated, false if not found
     */
    bool update_note(const Note& note, const crypto::SecureKey& subkey);

    /**
     * @brief Delete a note by ID
     * @param id Note ID to delete
     * @return true if note was found and deleted, false if not found
     */
    bool delete_note(int64_t id);

    // === Database Management ===

    void close();
    bool is_open() const;

private:
    sqlite3* db_;
    std::string db_path_;

    // Serialization helpers
    static std::vector<uint8_t> serialize_note(const Note& note);
    static std::optional<Note> deserialize_note(const std::vector<uint8_t>& json_bytes);

    // AAD construction (4 bytes little-endian note_id)
    static std::vector<uint8_t> build_aad(int64_t note_id);

    // Current UNIX timestamp
    static int64_t current_timestamp();
};

}  // namespace storage
}  // namespace bastionx

#endif  // BASTIONX_STORAGE_NOTESREPOSITORY_H
