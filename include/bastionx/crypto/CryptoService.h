#ifndef BASTIONX_CRYPTO_CRYPTOSERVICE_H
#define BASTIONX_CRYPTO_CRYPTOSERVICE_H

#include "bastionx/crypto/SecureMemory.h"
#include <sodium.h>
#include <array>
#include <vector>
#include <string>
#include <optional>
#include <cstdint>

namespace bastionx {
namespace crypto {

/**
 * @brief Central cryptographic service using libsodium exclusively
 *
 * CryptoService provides all cryptographic operations for Bastionx:
 * - Key derivation from passwords (Argon2id)
 * - Subkey derivation for different purposes (KDF)
 * - Authenticated encryption (XChaCha20-Poly1305 AEAD)
 * - Authenticated decryption with AAD validation
 *
 * All operations use libsodium primitives exclusively - no custom crypto.
 * This class is static-only and not instantiable.
 */
class CryptoService {
public:
    // === Cryptographic Constants (from CLAUDE.md) ===

    /// Salt size for Argon2id key derivation (16 bytes)
    static constexpr size_t SALT_BYTES = 16;

    /// Master key size (32 bytes for crypto_kdf_KEYBYTES)
    static constexpr size_t KEY_BYTES = crypto_kdf_KEYBYTES;

    /// Nonce size for XChaCha20-Poly1305 (24 bytes)
    static constexpr size_t NONCE_BYTES = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

    /// Subkey size (32 bytes, must match XChaCha20-Poly1305 key size)
    static constexpr size_t SUBKEY_BYTES = crypto_aead_xchacha20poly1305_ietf_KEYBYTES;

    /// Context string for KDF (must be exactly 8 bytes)
    static constexpr char KDF_CONTEXT[9] = "BastionX";

    // === Subkey Contexts ===

    /// Subkey context for note encryption/decryption
    static constexpr uint64_t SUBKEY_NOTES = 1;

    /// Subkey context for settings encryption (future use)
    static constexpr uint64_t SUBKEY_SETTINGS = 2;

    // === Data Structures ===

    /**
     * @brief Result of key derivation containing master key and salt
     */
    struct DerivedKey {
        SecureKey master_key;                       ///< 32-byte master key (secure memory)
        std::array<uint8_t, SALT_BYTES> salt;       ///< 16-byte salt
    };

    /**
     * @brief Encrypted data with nonce and ciphertext
     */
    struct EncryptedData {
        std::vector<uint8_t> ciphertext;            ///< Ciphertext + MAC tag
        std::array<uint8_t, NONCE_BYTES> nonce;     ///< 24-byte random nonce
    };

    // === Key Derivation ===

    /**
     * @brief Derive master key from password using Argon2id
     *
     * Uses libsodium's crypto_pwhash() with:
     * - Algorithm: Argon2id (ARGON2ID13)
     * - OpsLimit: MODERATE (protects against offline attacks)
     * - MemLimit: MODERATE (balances security and usability)
     *
     * @param password User password (UTF-8 string)
     * @param salt Optional salt (16 bytes). If nullopt, a random salt is generated
     * @return DerivedKey containing 32-byte master key and the salt used
     * @throws std::runtime_error if key derivation fails (out of memory)
     *
     * @note Derivation may take 100-500ms depending on hardware (intentional)
     * @warning Never reuse salts - always generate a new salt for new vaults
     */
    static DerivedKey derive_master_key(
        const std::string& password,
        const std::optional<std::array<uint8_t, SALT_BYTES>>& salt = std::nullopt
    );

    /**
     * @brief Derive subkey from master key using KDF
     *
     * Uses libsodium's crypto_kdf_derive_from_key() to derive context-specific
     * subkeys from the master key. This prevents cross-use of cryptographic
     * material between different purposes (notes, settings, etc).
     *
     * @param master_key 32-byte master key (from derive_master_key)
     * @param context Subkey context (e.g., SUBKEY_NOTES, SUBKEY_SETTINGS)
     * @return SecureKey containing 16-byte subkey
     * @throws std::invalid_argument if master_key size is incorrect
     */
    static SecureKey derive_subkey(
        const SecureKey& master_key,
        uint64_t context
    );

    // === Encryption / Decryption ===

    /**
     * @brief Encrypt plaintext using XChaCha20-Poly1305 AEAD
     *
     * Uses libsodium's crypto_aead_xchacha20poly1305_ietf_encrypt():
     * - Algorithm: XChaCha20 stream cipher + Poly1305 MAC
     * - Nonce: 24 bytes (randomly generated per encryption)
     * - AAD: Additional authenticated data (not encrypted, but authenticated)
     *
     * @param plaintext Data to encrypt (note payload, settings, etc)
     * @param subkey Subkey for this context (from derive_subkey)
     * @param associated_data AAD (e.g., note_id + timestamp) for ciphertext binding
     * @return EncryptedData containing ciphertext+MAC and nonce
     *
     * @note Ciphertext size = plaintext size + 16 bytes (Poly1305 MAC tag)
     * @note Nonce is randomly generated - never reuse keys without unique nonces
     */
    static EncryptedData encrypt(
        const std::vector<uint8_t>& plaintext,
        const SecureKey& subkey,
        const std::vector<uint8_t>& associated_data
    );

    /**
     * @brief Decrypt ciphertext using XChaCha20-Poly1305 AEAD
     *
     * Uses libsodium's crypto_aead_xchacha20poly1305_ietf_decrypt():
     * - Verifies MAC tag (authentication)
     * - Validates AAD matches (prevents ciphertext swapping)
     * - Decrypts ciphertext to plaintext
     *
     * @param encrypted EncryptedData containing ciphertext+MAC and nonce
     * @param subkey Subkey for this context (same as used for encryption)
     * @param associated_data AAD (must match encryption AAD exactly)
     * @return std::optional<std::vector<uint8_t>> plaintext on success, nullopt on failure
     *
     * @note Returns nullopt if:
     *       - MAC verification fails (wrong key, tampered ciphertext)
     *       - AAD mismatch (ciphertext swapped to different context)
     *       - Corrupted ciphertext
     */
    static std::optional<std::vector<uint8_t>> decrypt(
        const EncryptedData& encrypted,
        const SecureKey& subkey,
        const std::vector<uint8_t>& associated_data
    );

private:
    // Static-only class - prevent instantiation
    CryptoService() = delete;
    ~CryptoService() = delete;
    CryptoService(const CryptoService&) = delete;
    CryptoService& operator=(const CryptoService&) = delete;
};

}  // namespace crypto
}  // namespace bastionx

#endif  // BASTIONX_CRYPTO_CRYPTOSERVICE_H
