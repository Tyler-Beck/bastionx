#include "bastionx/crypto/CryptoService.h"
#include <stdexcept>
#include <cstring>

namespace bastionx {
namespace crypto {

// === Key Derivation Implementation ===

CryptoService::DerivedKey CryptoService::derive_master_key(
    const std::string& password,
    const std::optional<std::array<uint8_t, SALT_BYTES>>& salt
) {
    // Allocate secure memory for master key upfront
    DerivedKey result{SecureKey(KEY_BYTES), {}};

    // Generate or use provided salt
    if (salt.has_value()) {
        result.salt = *salt;
    } else {
        // Generate random salt using libsodium's CSPRNG
        randombytes_buf(result.salt.data(), SALT_BYTES);
    }

    // Derive key using Argon2id with MODERATE parameters
    // This balances security (offline attack resistance) with usability (100-500ms)
    int rc = crypto_pwhash(
        result.master_key.data(),
        KEY_BYTES,
        password.data(),
        password.size(),
        result.salt.data(),
        crypto_pwhash_OPSLIMIT_MODERATE,
        crypto_pwhash_MEMLIMIT_MODERATE,
        crypto_pwhash_ALG_ARGON2ID13
    );

    if (rc != 0) {
        // Derivation failed - likely out of memory
        throw std::runtime_error("Key derivation failed (insufficient memory)");
    }

    return result;
}

// === Subkey Derivation Implementation ===

SecureKey CryptoService::derive_subkey(
    const SecureKey& master_key,
    uint64_t context
) {
    // Validate master key size
    if (master_key.size() != crypto_kdf_KEYBYTES) {
        throw std::invalid_argument(
            "Invalid master key size: expected " +
            std::to_string(crypto_kdf_KEYBYTES) +
            " bytes, got " +
            std::to_string(master_key.size())
        );
    }

    // Allocate secure memory for subkey
    SecureKey subkey(SUBKEY_BYTES);

    // Derive subkey using libsodium KDF
    // Context string must be exactly 8 bytes (enforced by libsodium)
    crypto_kdf_derive_from_key(
        subkey.data(),
        subkey.size(),
        context,
        KDF_CONTEXT,
        master_key.data()
    );

    return subkey;
}

// === Encryption Implementation ===

CryptoService::EncryptedData CryptoService::encrypt(
    const std::vector<uint8_t>& plaintext,
    const SecureKey& subkey,
    const std::vector<uint8_t>& associated_data
) {
    EncryptedData result;

    // Generate random nonce (24 bytes for XChaCha20-Poly1305)
    randombytes_buf(result.nonce.data(), NONCE_BYTES);

    // Allocate ciphertext buffer
    // Size = plaintext + MAC tag (16 bytes for Poly1305)
    result.ciphertext.resize(
        plaintext.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES
    );

    unsigned long long ciphertext_len;

    // Encrypt using XChaCha20-Poly1305 AEAD
    crypto_aead_xchacha20poly1305_ietf_encrypt(
        result.ciphertext.data(),           // output: ciphertext + tag
        &ciphertext_len,                    // output: actual ciphertext length
        plaintext.data(),                   // input: plaintext
        plaintext.size(),                   // input: plaintext length
        associated_data.data(),             // AAD: additional authenticated data
        associated_data.size(),             // AAD length
        nullptr,                            // nsec (not used in this variant)
        result.nonce.data(),                // nonce (24 bytes, random)
        subkey.data()                       // key (32 bytes)
    );

    // Resize to actual length (should match allocated size)
    result.ciphertext.resize(ciphertext_len);

    return result;
}

// === Decryption Implementation ===

std::optional<std::vector<uint8_t>> CryptoService::decrypt(
    const EncryptedData& encrypted,
    const SecureKey& subkey,
    const std::vector<uint8_t>& associated_data
) {
    // Allocate plaintext buffer
    // Size = ciphertext - MAC tag
    std::vector<uint8_t> plaintext(encrypted.ciphertext.size());
    unsigned long long plaintext_len;

    // Decrypt using XChaCha20-Poly1305 AEAD
    int rc = crypto_aead_xchacha20poly1305_ietf_decrypt(
        plaintext.data(),                   // output: plaintext
        &plaintext_len,                     // output: actual plaintext length
        nullptr,                            // nsec (not used in this variant)
        encrypted.ciphertext.data(),        // input: ciphertext + tag
        encrypted.ciphertext.size(),        // input: ciphertext length
        associated_data.data(),             // AAD: must match encryption AAD
        associated_data.size(),             // AAD length
        encrypted.nonce.data(),             // nonce (must match encryption nonce)
        subkey.data()                       // key (must match encryption key)
    );

    if (rc != 0) {
        // Decryption failed - either:
        // 1. MAC verification failed (wrong key or tampered ciphertext)
        // 2. AAD mismatch (ciphertext swapped to different context)
        // 3. Corrupted ciphertext
        return std::nullopt;
    }

    // Resize to actual plaintext length
    plaintext.resize(plaintext_len);

    return plaintext;
}

}  // namespace crypto
}  // namespace bastionx
