#include <gtest/gtest.h>
#include "bastionx/crypto/CryptoService.h"
#include "test_vectors.h"
#include <sodium.h>
#include <chrono>

using namespace bastionx::crypto;

/**
 * @brief Test fixture for CryptoService tests
 */
class CryptoServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // sodium_init() is called once in test_main.cpp
    }
};

// ===================================================================
// Test 1: Key derivation produces deterministic output
// ===================================================================
TEST_F(CryptoServiceTest, KeyDerivationDeterministic) {
    const std::string password = "test_password_123";
    const auto salt = std::array<uint8_t, 16>{
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10
    };

    // Derive key twice with same password and salt
    auto key1 = CryptoService::derive_master_key(password, salt);
    auto key2 = CryptoService::derive_master_key(password, salt);

    // Keys should be identical
    ASSERT_EQ(key1.master_key.size(), key2.master_key.size());
    EXPECT_EQ(0, sodium_memcmp(
        key1.master_key.data(),
        key2.master_key.data(),
        key1.master_key.size()
    )) << "Key derivation should be deterministic";

    // Salts should also match
    EXPECT_EQ(key1.salt, key2.salt);
}

// ===================================================================
// Test 2: Different passwords produce different keys
// ===================================================================
TEST_F(CryptoServiceTest, DifferentPasswordsDifferentKeys) {
    const auto salt = std::array<uint8_t, 16>{
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10
    };

    auto key1 = CryptoService::derive_master_key("password1", salt);
    auto key2 = CryptoService::derive_master_key("password2", salt);

    // Keys should be different
    EXPECT_NE(0, sodium_memcmp(
        key1.master_key.data(),
        key2.master_key.data(),
        key1.master_key.size()
    )) << "Different passwords should produce different keys";
}

// ===================================================================
// Test 3: Different salts produce different keys
// ===================================================================
TEST_F(CryptoServiceTest, DifferentSaltsDifferentKeys) {
    const std::string password = "test_password";

    auto key1 = CryptoService::derive_master_key(password,
        std::array<uint8_t, 16>{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10});
    auto key2 = CryptoService::derive_master_key(password,
        std::array<uint8_t, 16>{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20});

    // Keys should be different
    EXPECT_NE(0, sodium_memcmp(
        key1.master_key.data(),
        key2.master_key.data(),
        key1.master_key.size()
    )) << "Different salts should produce different keys";
}

// ===================================================================
// Test 4: Subkey derivation creates unique keys per context
// ===================================================================
TEST_F(CryptoServiceTest, SubkeyDerivation) {
    auto master = CryptoService::derive_master_key("test", std::nullopt);

    auto subkey_notes = CryptoService::derive_subkey(
        master.master_key,
        CryptoService::SUBKEY_NOTES
    );
    auto subkey_settings = CryptoService::derive_subkey(
        master.master_key,
        CryptoService::SUBKEY_SETTINGS
    );

    // Different contexts should produce different subkeys
    EXPECT_NE(0, sodium_memcmp(
        subkey_notes.data(),
        subkey_settings.data(),
        std::min(subkey_notes.size(), subkey_settings.size())
    )) << "Different contexts should produce different subkeys";

    // Subkeys should be deterministic
    auto subkey_notes2 = CryptoService::derive_subkey(
        master.master_key,
        CryptoService::SUBKEY_NOTES
    );

    EXPECT_EQ(0, sodium_memcmp(
        subkey_notes.data(),
        subkey_notes2.data(),
        subkey_notes.size()
    )) << "Subkey derivation should be deterministic";
}

// ===================================================================
// Test 5: Encryption/decryption round-trip succeeds
// ===================================================================
TEST_F(CryptoServiceTest, EncryptDecryptRoundTrip) {
    const std::string plaintext_str = "This is a secret note";
    const std::vector<uint8_t> plaintext(
        plaintext_str.begin(),
        plaintext_str.end()
    );

    auto master = CryptoService::derive_master_key("password", std::nullopt);
    auto subkey = CryptoService::derive_subkey(
        master.master_key,
        CryptoService::SUBKEY_NOTES
    );

    // Associated data: note ID (4 bytes, little-endian)
    const std::vector<uint8_t> aad = {0x01, 0x00, 0x00, 0x00};  // note_id=1

    // Encrypt
    auto encrypted = CryptoService::encrypt(plaintext, subkey, aad);

    // Verify ciphertext size (plaintext + 16 byte MAC)
    EXPECT_EQ(plaintext.size() + 16, encrypted.ciphertext.size());

    // Decrypt
    auto decrypted = CryptoService::decrypt(encrypted, subkey, aad);

    // Verify decryption succeeded
    ASSERT_TRUE(decrypted.has_value()) << "Decryption should succeed";

    // Verify plaintext matches
    EXPECT_EQ(plaintext, *decrypted) << "Decrypted plaintext should match original";
}

// ===================================================================
// Test 6: Decryption with wrong key fails
// ===================================================================
TEST_F(CryptoServiceTest, DecryptionWithWrongKeyFails) {
    const std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03};

    auto key1 = CryptoService::derive_master_key("password1", std::nullopt);
    auto key2 = CryptoService::derive_master_key("password2", std::nullopt);

    auto subkey1 = CryptoService::derive_subkey(key1.master_key, 1);
    auto subkey2 = CryptoService::derive_subkey(key2.master_key, 1);

    // Encrypt with key1
    auto encrypted = CryptoService::encrypt(plaintext, subkey1, {});

    // Attempt to decrypt with key2
    auto decrypted = CryptoService::decrypt(encrypted, subkey2, {});

    // Decryption should fail (wrong key)
    EXPECT_FALSE(decrypted.has_value()) << "Decryption with wrong key should fail";
}

// ===================================================================
// Test 7: AAD mismatch prevents decryption
// ===================================================================
TEST_F(CryptoServiceTest, AADMismatchFails) {
    const std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03};
    const std::vector<uint8_t> aad1 = {0x01};  // note_id=1
    const std::vector<uint8_t> aad2 = {0x02};  // note_id=2

    auto master = CryptoService::derive_master_key("password", std::nullopt);
    auto subkey = CryptoService::derive_subkey(master.master_key, 1);

    // Encrypt with aad1
    auto encrypted = CryptoService::encrypt(plaintext, subkey, aad1);

    // Attempt to decrypt with aad2
    auto decrypted = CryptoService::decrypt(encrypted, subkey, aad2);

    // Decryption should fail (AAD mismatch)
    EXPECT_FALSE(decrypted.has_value()) << "Decryption with wrong AAD should fail";
}

// ===================================================================
// Test 8: Empty plaintext encryption works
// ===================================================================
TEST_F(CryptoServiceTest, EmptyPlaintext) {
    const std::vector<uint8_t> plaintext = {};

    auto master = CryptoService::derive_master_key("password", std::nullopt);
    auto subkey = CryptoService::derive_subkey(master.master_key, 1);

    // Encrypt empty plaintext
    auto encrypted = CryptoService::encrypt(plaintext, subkey, {});

    // Ciphertext should be just the MAC tag (16 bytes)
    EXPECT_EQ(16, encrypted.ciphertext.size());

    // Decrypt
    auto decrypted = CryptoService::decrypt(encrypted, subkey, {});

    // Verify decryption succeeded
    ASSERT_TRUE(decrypted.has_value()) << "Decryption of empty plaintext should succeed";

    // Verify decrypted plaintext is empty
    EXPECT_TRUE(decrypted->empty()) << "Decrypted empty plaintext should be empty";
}

// ===================================================================
// Test 9: Large plaintext (10KB) encryption works
// ===================================================================
TEST_F(CryptoServiceTest, LargePlaintext) {
    // Generate 10 KB of random data
    std::vector<uint8_t> plaintext(10 * 1024);
    randombytes_buf(plaintext.data(), plaintext.size());

    auto master = CryptoService::derive_master_key("password", std::nullopt);
    auto subkey = CryptoService::derive_subkey(master.master_key, 1);

    // Encrypt
    auto encrypted = CryptoService::encrypt(plaintext, subkey, {});

    // Verify ciphertext size
    EXPECT_EQ(plaintext.size() + 16, encrypted.ciphertext.size());

    // Decrypt
    auto decrypted = CryptoService::decrypt(encrypted, subkey, {});

    // Verify decryption succeeded
    ASSERT_TRUE(decrypted.has_value()) << "Decryption of large plaintext should succeed";

    // Verify plaintext matches
    EXPECT_EQ(plaintext, *decrypted) << "Decrypted large plaintext should match original";
}

// ===================================================================
// Test 10: Ciphertext tampering is detected
// ===================================================================
TEST_F(CryptoServiceTest, CiphertextTamperingDetected) {
    const std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03};

    auto master = CryptoService::derive_master_key("password", std::nullopt);
    auto subkey = CryptoService::derive_subkey(master.master_key, 1);

    // Encrypt
    auto encrypted = CryptoService::encrypt(plaintext, subkey, {});

    // Tamper with ciphertext (flip first byte)
    if (!encrypted.ciphertext.empty()) {
        encrypted.ciphertext[0] ^= 0xFF;
    }

    // Attempt to decrypt
    auto decrypted = CryptoService::decrypt(encrypted, subkey, {});

    // Decryption should fail (tampering detected via MAC)
    EXPECT_FALSE(decrypted.has_value()) << "Decryption of tampered ciphertext should fail";
}

// ===================================================================
// Bonus Test: Performance benchmark for key derivation
// ===================================================================
TEST_F(CryptoServiceTest, KeyDerivationPerformance) {
    const std::string password = "test_password";

    auto start = std::chrono::high_resolution_clock::now();
    auto key = CryptoService::derive_master_key(password, std::nullopt);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Key derivation time: " << duration.count() << " ms\n";

    // Should complete in reasonable time (< 2 seconds for MODERATE settings)
    EXPECT_LT(duration.count(), 2000)
        << "Key derivation should complete in under 2 seconds";
}
