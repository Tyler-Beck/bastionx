#ifndef BASTIONX_TESTS_CRYPTO_TEST_VECTORS_H
#define BASTIONX_TESTS_CRYPTO_TEST_VECTORS_H

#include <array>
#include <vector>
#include <cstdint>
#include <string>

/**
 * @file test_vectors.h
 * @brief Known-good test vectors for cryptographic validation
 *
 * Test vectors ensure that cryptographic implementations produce expected
 * outputs for given inputs. These vectors can be cross-validated against
 * reference implementations or other libsodium-based applications.
 */

namespace test_vectors {

/**
 * @brief Test vector for key derivation (Argon2id)
 */
struct KDFVector {
    std::string password;
    std::array<uint8_t, 16> salt;
    std::array<uint8_t, 32> expected_key;
    std::string description;
};

/**
 * @brief Test vector for AEAD encryption (XChaCha20-Poly1305)
 */
struct AEADVector {
    std::vector<uint8_t> plaintext;
    std::vector<uint8_t> key;
    std::array<uint8_t, 24> nonce;
    std::vector<uint8_t> associated_data;
    std::vector<uint8_t> expected_ciphertext;
    std::string description;
};

// ===================================================================
// Key Derivation Test Vectors
// ===================================================================

/**
 * @brief Test vector: Empty password
 *
 * Tests edge case of zero-length password
 */
inline const KDFVector kdf_empty_password = {
    .password = "",
    .salt = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    },
    .expected_key = {
        // NOTE: This should be precomputed using a reference implementation
        // For now, this is a placeholder - tests will compute dynamically
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    .description = "Empty password (edge case)"
};

/**
 * @brief Test vector: ASCII password
 *
 * Tests common case of ASCII password
 */
inline const KDFVector kdf_ascii_password = {
    .password = "correct horse battery staple",
    .salt = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    },
    .expected_key = {
        // NOTE: This should be precomputed using a reference implementation
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    .description = "ASCII password (xkcd style passphrase)"
};

/**
 * @brief Test vector: UTF-8 password
 *
 * Tests Unicode password handling
 */
inline const KDFVector kdf_utf8_password = {
    .password = "пароль123",  // Russian + numbers
    .salt = {
        0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
    },
    .expected_key = {
        // NOTE: This should be precomputed using a reference implementation
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    .description = "UTF-8 password (Cyrillic characters)"
};

// ===================================================================
// AEAD Test Vectors (XChaCha20-Poly1305)
// ===================================================================

/**
 * @brief Test vector: Empty plaintext
 *
 * Tests edge case of zero-length plaintext
 */
inline const AEADVector aead_empty_plaintext = {
    .plaintext = {},
    .key = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f
    },
    .nonce = {
        0x07, 0x00, 0x00, 0x00, 0x40, 0x41, 0x42, 0x43,
        0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    .associated_data = {},
    .expected_ciphertext = {
        // NOTE: Only MAC tag (16 bytes) for empty plaintext
        // Should be precomputed
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    .description = "Empty plaintext (MAC tag only)"
};

/**
 * @brief Test vector: Simple plaintext with AAD
 *
 * Tests typical use case with both plaintext and AAD
 */
inline const AEADVector aead_simple_with_aad = {
    .plaintext = {0x4c, 0x61, 0x64, 0x69, 0x65, 0x73, 0x20, 0x61},  // "Ladies a"
    .key = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f
    },
    .nonce = {
        0x07, 0x00, 0x00, 0x00, 0x40, 0x41, 0x42, 0x43,
        0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    .associated_data = {0x50, 0x51, 0x52, 0x53},  // AAD: 4 bytes
    .expected_ciphertext = {
        // NOTE: Should be precomputed (plaintext_len + 16 bytes MAC)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    .description = "Simple plaintext with AAD"
};

// ===================================================================
// Helper Functions
// ===================================================================

/**
 * @brief Compare two byte arrays for equality
 * @param a First array
 * @param b Second array
 * @param len Length to compare
 * @return true if arrays are equal, false otherwise
 */
inline bool compare_bytes(const uint8_t* a, const uint8_t* b, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

}  // namespace test_vectors

#endif  // BASTIONX_TESTS_CRYPTO_TEST_VECTORS_H
