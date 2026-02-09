#ifndef BASTIONX_VAULT_VAULTSERVICE_H
#define BASTIONX_VAULT_VAULTSERVICE_H

#include "bastionx/crypto/CryptoService.h"
#include "bastionx/crypto/SecureMemory.h"
#include <sqlite3.h>
#include <string>
#include <array>
#include <optional>
#include <cstdint>

namespace bastionx {
namespace vault {

/**
 * @brief Vault lifecycle states
 */
enum class VaultState {
    kNoVault,   ///< No vault file exists at the path
    kLocked,    ///< Vault file exists, master key not in memory
    kUnlocked   ///< Vault is open, master key and subkeys in memory
};

/**
 * @brief Manages vault lifecycle, password verification, and key material
 *
 * VaultService is responsible for:
 * - Creating new vaults (password → key derivation → schema + verification token)
 * - Unlocking existing vaults (password verification via encrypted token)
 * - Locking vaults (wiping all key material from memory)
 * - Providing subkeys to NotesRepository for CRUD operations
 *
 * Key material is stored in std::optional<SecureKey>. Locking resets these
 * optionals, which triggers SecureBuffer's destructor (sodium_memzero + sodium_free).
 */
class VaultService {
public:
    /// Known-plaintext marker for password verification (32 bytes)
    static constexpr char VERIFY_MARKER[33] = "BASTIONX_VAULT_VERIFY_OK_MARKER";
    static constexpr size_t VERIFY_MARKER_SIZE = 32;

    /**
     * @brief Construct VaultService for a given vault file path
     * @param vault_path Path to the SQLite vault database file
     */
    explicit VaultService(const std::string& vault_path);
    ~VaultService();

    // Non-copyable, non-movable (owns sensitive key material)
    VaultService(const VaultService&) = delete;
    VaultService& operator=(const VaultService&) = delete;
    VaultService(VaultService&&) = delete;
    VaultService& operator=(VaultService&&) = delete;

    // === Vault Lifecycle ===

    /**
     * @brief Create a new vault with the given password
     *
     * Creates the SQLite database file, schema, stores salt + KDF params,
     * and stores the password verification token.
     *
     * @param password User-chosen master password
     * @return true if vault created successfully, false if vault already exists
     * @throws std::runtime_error on SQLite errors
     *
     * @note Transitions state: kNoVault → kUnlocked
     */
    bool create(const std::string& password);

    /**
     * @brief Unlock an existing vault by verifying the password
     *
     * Loads salt from DB, derives master key, verifies via encrypted token.
     *
     * @param password User-provided password
     * @return true if password correct and vault unlocked, false if wrong password
     * @throws std::runtime_error on SQLite errors or if vault doesn't exist
     *
     * @note Transitions state: kLocked → kUnlocked
     */
    bool unlock(const std::string& password);

    /**
     * @brief Lock the vault and wipe all key material from memory
     *
     * @note Transitions state: kUnlocked → kLocked
     */
    void lock();

    // === State Queries ===

    VaultState state() const;
    bool is_unlocked() const;

    // === Key Access ===

    /**
     * @brief Get the notes subkey for NotesRepository operations
     * @return Const reference to the notes subkey
     * @throws std::runtime_error if vault is locked
     */
    const crypto::SecureKey& notes_subkey() const;

    /**
     * @brief Get the settings subkey
     * @return Const reference to the settings subkey
     * @throws std::runtime_error if vault is locked
     */
    const crypto::SecureKey& settings_subkey() const;

    // === Settings Persistence ===

    /**
     * @brief Save encrypted settings JSON to the vault
     * @param json_str JSON string from VaultSettings::to_json()
     * @throws std::runtime_error if vault is locked or on SQLite errors
     */
    void save_settings(const std::string& json_str);

    /**
     * @brief Load and decrypt settings JSON from the vault
     * @return JSON string, or empty string if no settings stored
     * @throws std::runtime_error if vault is locked or on SQLite errors
     */
    std::string load_settings();

    // === Password Change ===

    /**
     * @brief Change the vault master password
     *
     * Atomically re-encrypts all notes, verify token, and settings with new key
     * material derived from the new password. Uses an EXCLUSIVE transaction so
     * that on any failure the vault remains usable with the old password.
     *
     * @param current_password Current password (re-verified for safety)
     * @param new_password New password
     * @return true if password changed, false if current password wrong
     * @throws std::runtime_error if vault is locked or on SQLite errors
     */
    bool change_password(const std::string& current_password,
                         const std::string& new_password);

    /**
     * @brief Get the vault file path
     */
    const std::string& vault_path() const;

private:
    std::string vault_path_;
    VaultState state_;

    // Key material (only valid when state_ == kUnlocked)
    std::optional<crypto::SecureKey> master_key_;
    std::optional<crypto::SecureKey> notes_subkey_;
    std::optional<crypto::SecureKey> verify_subkey_;
    std::optional<crypto::SecureKey> settings_subkey_;

    // Cached vault metadata
    std::array<uint8_t, crypto::CryptoService::SALT_BYTES> salt_{};
    uint64_t kdf_opslimit_ = 0;
    uint64_t kdf_memlimit_ = 0;

    // Internal helpers
    void wipe_keys();
    bool verify_password();
    void create_schema(sqlite3* db);
    void migrate_schema(sqlite3* db);
    void store_vault_meta(sqlite3* db);
    void store_verify_token(sqlite3* db);
    bool load_vault_meta(sqlite3* db);
    bool load_verify_token(sqlite3* db,
                           std::array<uint8_t, crypto::CryptoService::NONCE_BYTES>& nonce,
                           std::vector<uint8_t>& ciphertext);
};

}  // namespace vault
}  // namespace bastionx

#endif  // BASTIONX_VAULT_VAULTSERVICE_H
