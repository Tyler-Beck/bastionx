#include <gtest/gtest.h>
#include "bastionx/vault/VaultService.h"
#include "bastionx/storage/NotesRepository.h"
#include <sodium.h>
#include <filesystem>
#include <fstream>

using namespace bastionx::vault;
using namespace bastionx::storage;
using namespace bastionx::crypto;
namespace fs = std::filesystem;

/**
 * @brief SQLCipher verification tests
 *
 * These tests verify that full database encryption is working correctly:
 * - Database file is opaque (no plaintext SQLite header)
 * - Wrong key cannot open the database
 * - Correct key opens successfully
 * - Salt sidecar file is created and matches vault_meta
 */
class SQLCipherTest : public ::testing::Test {
protected:
    std::string vault_path_;
    std::string temp_dir_;

    void SetUp() override {
        unsigned char buf[8];
        randombytes_buf(buf, sizeof(buf));
        std::string suffix;
        for (auto b : buf) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", b);
            suffix += hex;
        }

        temp_dir_ = (fs::temp_directory_path() / ("bastionx_sqlcipher_test_" + suffix)).string();
        fs::create_directories(temp_dir_);
        vault_path_ = (fs::path(temp_dir_) / "vault.db").string();
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(temp_dir_, ec);
    }
};

// ===================================================================
// Test 1: Database file is opaque (no plaintext SQLite header)
// ===================================================================
TEST_F(SQLCipherTest, DatabaseFileIsOpaque) {
    // Create a vault with a note
    VaultService vault(vault_path_);
    ASSERT_TRUE(vault.create("test_password"));

    NotesRepository repo(vault_path_, &vault.db_subkey());
    Note n;
    n.title = "Secret Note";
    n.body = "This is highly confidential content";
    repo.create_note(n, vault.notes_subkey());
    repo.close();
    vault.lock();

    // Read first 16 bytes of the database file
    std::ifstream f(vault_path_, std::ios::binary);
    ASSERT_TRUE(f.good());

    char header[16];
    f.read(header, sizeof(header));
    ASSERT_EQ(f.gcount(), 16);
    f.close();

    // SQLite plaintext header is "SQLite format 3\0"
    const char sqlite_header[] = "SQLite format 3";
    EXPECT_NE(std::memcmp(header, sqlite_header, 16), 0)
        << "Database file starts with plaintext SQLite header — encryption not working!";

    // Also verify no plaintext strings appear in the file
    std::ifstream raw(vault_path_, std::ios::binary);
    std::string contents((std::istreambuf_iterator<char>(raw)),
                          std::istreambuf_iterator<char>());
    raw.close();

    EXPECT_EQ(contents.find("Secret Note"), std::string::npos)
        << "Plaintext note title found in database file";
    EXPECT_EQ(contents.find("highly confidential"), std::string::npos)
        << "Plaintext note body found in database file";
    EXPECT_EQ(contents.find("vault_meta"), std::string::npos)
        << "Plaintext table name found in database file";
    EXPECT_EQ(contents.find("CREATE TABLE"), std::string::npos)
        << "Plaintext SQL found in database file";
}

// ===================================================================
// Test 2: Wrong key cannot open the database
// ===================================================================
TEST_F(SQLCipherTest, WrongKeyCannotOpen) {
    // Create vault
    VaultService vault(vault_path_);
    ASSERT_TRUE(vault.create("test_password"));
    vault.lock();

    // Try to open the DB with a random (wrong) key
    sqlite3* db = nullptr;
    int rc = sqlite3_open(vault_path_.c_str(), &db);
    ASSERT_EQ(rc, SQLITE_OK);

    // Apply a wrong key
    unsigned char wrong_key[32];
    randombytes_buf(wrong_key, sizeof(wrong_key));
    rc = sqlite3_key(db, wrong_key, sizeof(wrong_key));
    EXPECT_EQ(rc, SQLITE_OK);  // sqlite3_key itself succeeds

    // First query should fail (wrong key → corrupt decryption)
    char* err_msg = nullptr;
    rc = sqlite3_exec(db, "SELECT count(*) FROM vault_meta;",
                      nullptr, nullptr, &err_msg);
    EXPECT_NE(rc, SQLITE_OK)
        << "Query succeeded with wrong key — encryption not working!";

    if (err_msg) sqlite3_free(err_msg);
    sqlite3_close(db);
}

// ===================================================================
// Test 3: Correct key opens successfully
// ===================================================================
TEST_F(SQLCipherTest, CorrectKeyOpens) {
    // Create vault
    VaultService vault(vault_path_);
    ASSERT_TRUE(vault.create("test_password"));

    // Get the db subkey before locking
    // Copy the key data for use after lock
    std::vector<uint8_t> key_copy(vault.db_subkey().data(),
                                   vault.db_subkey().data() + vault.db_subkey().size());
    vault.lock();

    // Open with the correct key
    sqlite3* db = nullptr;
    int rc = sqlite3_open(vault_path_.c_str(), &db);
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_key(db, key_copy.data(), static_cast<int>(key_copy.size()));
    ASSERT_EQ(rc, SQLITE_OK);

    // Query should succeed
    char* err_msg = nullptr;
    rc = sqlite3_exec(db, "SELECT count(*) FROM vault_meta;",
                      nullptr, nullptr, &err_msg);
    EXPECT_EQ(rc, SQLITE_OK)
        << "Query failed with correct key: " << (err_msg ? err_msg : "unknown");

    if (err_msg) sqlite3_free(err_msg);
    sqlite3_close(db);
}

// ===================================================================
// Test 4: Salt sidecar file is created with correct size
// ===================================================================
TEST_F(SQLCipherTest, SaltFileCreated) {
    VaultService vault(vault_path_);
    ASSERT_TRUE(vault.create("test_password"));

    // Check that vault.salt exists
    auto salt_path = fs::path(temp_dir_) / "vault.salt";
    EXPECT_TRUE(fs::exists(salt_path))
        << "Salt sidecar file not created";

    // Check exact size (16 bytes = CryptoService::SALT_BYTES)
    auto file_size = fs::file_size(salt_path);
    EXPECT_EQ(file_size, CryptoService::SALT_BYTES)
        << "Salt file should be exactly " << CryptoService::SALT_BYTES << " bytes";
}

// ===================================================================
// Test 5: Salt sidecar matches vault_meta table
// ===================================================================
TEST_F(SQLCipherTest, SaltSidecarMatchesVaultMeta) {
    VaultService vault(vault_path_);
    ASSERT_TRUE(vault.create("test_password"));

    // Read salt from sidecar file
    auto salt_path = fs::path(temp_dir_) / "vault.salt";
    std::ifstream f(salt_path.string(), std::ios::binary);
    ASSERT_TRUE(f.good());
    std::array<uint8_t, CryptoService::SALT_BYTES> file_salt{};
    f.read(reinterpret_cast<char*>(file_salt.data()), file_salt.size());
    f.close();

    // Read salt from vault_meta via direct DB access
    sqlite3* db = nullptr;
    int rc = sqlite3_open(vault_path_.c_str(), &db);
    ASSERT_EQ(rc, SQLITE_OK);
    rc = sqlite3_key(db, vault.db_subkey().data(),
                     static_cast<int>(vault.db_subkey().size()));
    ASSERT_EQ(rc, SQLITE_OK);

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, "SELECT salt FROM vault_meta LIMIT 1", -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);

    const void* db_salt_blob = sqlite3_column_blob(stmt, 0);
    int db_salt_size = sqlite3_column_bytes(stmt, 0);
    ASSERT_EQ(db_salt_size, static_cast<int>(CryptoService::SALT_BYTES));

    std::array<uint8_t, CryptoService::SALT_BYTES> db_salt{};
    std::memcpy(db_salt.data(), db_salt_blob, CryptoService::SALT_BYTES);

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    // Compare
    EXPECT_EQ(file_salt, db_salt)
        << "Salt sidecar does not match vault_meta salt";
}

// ===================================================================
// Test 6: No plaintext open (without key) succeeds
// ===================================================================
TEST_F(SQLCipherTest, PlaintextOpenFails) {
    VaultService vault(vault_path_);
    ASSERT_TRUE(vault.create("test_password"));
    vault.lock();

    // Open WITHOUT any key
    sqlite3* db = nullptr;
    int rc = sqlite3_open(vault_path_.c_str(), &db);
    ASSERT_EQ(rc, SQLITE_OK);

    // Any query should fail — the file is encrypted, not a valid plaintext SQLite DB
    char* err_msg = nullptr;
    rc = sqlite3_exec(db, "SELECT count(*) FROM vault_meta;",
                      nullptr, nullptr, &err_msg);
    EXPECT_NE(rc, SQLITE_OK)
        << "Query succeeded without key — database is not encrypted!";

    if (err_msg) sqlite3_free(err_msg);
    sqlite3_close(db);
}
