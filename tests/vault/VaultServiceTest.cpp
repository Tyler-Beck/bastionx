#include <gtest/gtest.h>
#include "bastionx/vault/VaultService.h"
#include <sodium.h>
#include <filesystem>
#include <string>

using namespace bastionx::vault;
using namespace bastionx::crypto;
namespace fs = std::filesystem;

/**
 * @brief Test fixture for VaultService tests
 *
 * Creates a unique temp directory for each test and cleans up after.
 */
class VaultServiceTest : public ::testing::Test {
protected:
    std::string vault_path_;
    std::string temp_dir_;

    void SetUp() override {
        // Create unique temp directory per test
        temp_dir_ = (fs::temp_directory_path() / ("bastionx_test_" + random_suffix())).string();
        fs::create_directories(temp_dir_);
        vault_path_ = (fs::path(temp_dir_) / "vault.db").string();
    }

    void TearDown() override {
        // Clean up temp files
        std::error_code ec;
        fs::remove_all(temp_dir_, ec);
    }

    static std::string random_suffix() {
        unsigned char buf[8];
        randombytes_buf(buf, sizeof(buf));
        std::string result;
        for (auto b : buf) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", b);
            result += hex;
        }
        return result;
    }
};

// ===================================================================
// Test 1: Create new vault
// ===================================================================
TEST_F(VaultServiceTest, CreateNewVault) {
    VaultService vault(vault_path_);

    EXPECT_EQ(VaultState::kNoVault, vault.state());

    bool created = vault.create("test_password");

    EXPECT_TRUE(created);
    EXPECT_EQ(VaultState::kUnlocked, vault.state());
    EXPECT_TRUE(vault.is_unlocked());
    EXPECT_TRUE(fs::exists(vault_path_));

    // notes_subkey should be accessible
    EXPECT_NO_THROW(vault.notes_subkey());
}

// ===================================================================
// Test 2: Create vault that already exists fails
// ===================================================================
TEST_F(VaultServiceTest, CreateVaultAlreadyExists) {
    VaultService vault(vault_path_);
    EXPECT_TRUE(vault.create("password1"));

    // Try creating again on same path
    VaultService vault2(vault_path_);
    EXPECT_FALSE(vault2.create("password2"));
}

// ===================================================================
// Test 3: Unlock with correct password
// ===================================================================
TEST_F(VaultServiceTest, UnlockCorrectPassword) {
    // Create vault
    {
        VaultService vault(vault_path_);
        vault.create("correct_password");
    }

    // Unlock with correct password
    VaultService vault(vault_path_);
    EXPECT_EQ(VaultState::kLocked, vault.state());

    bool unlocked = vault.unlock("correct_password");
    EXPECT_TRUE(unlocked);
    EXPECT_EQ(VaultState::kUnlocked, vault.state());
}

// ===================================================================
// Test 4: Unlock with wrong password fails
// ===================================================================
TEST_F(VaultServiceTest, UnlockWrongPassword) {
    // Create vault
    {
        VaultService vault(vault_path_);
        vault.create("correct_password");
    }

    // Unlock with wrong password
    VaultService vault(vault_path_);
    bool unlocked = vault.unlock("wrong_password");
    EXPECT_FALSE(unlocked);
    EXPECT_EQ(VaultState::kLocked, vault.state());
    EXPECT_FALSE(vault.is_unlocked());
}

// ===================================================================
// Test 5: Unlock nonexistent vault fails
// ===================================================================
TEST_F(VaultServiceTest, UnlockNonexistentVault) {
    std::string fake_path = (fs::path(temp_dir_) / "nonexistent.db").string();
    VaultService vault(fake_path);

    EXPECT_EQ(VaultState::kNoVault, vault.state());
    EXPECT_FALSE(vault.unlock("password"));
}

// ===================================================================
// Test 6: Lock wipes keys
// ===================================================================
TEST_F(VaultServiceTest, LockWipesKeys) {
    VaultService vault(vault_path_);
    vault.create("password");
    EXPECT_TRUE(vault.is_unlocked());

    vault.lock();
    EXPECT_EQ(VaultState::kLocked, vault.state());
    EXPECT_FALSE(vault.is_unlocked());
}

// ===================================================================
// Test 7: notes_subkey() throws when locked
// ===================================================================
TEST_F(VaultServiceTest, NotesSubkeyThrowsWhenLocked) {
    VaultService vault(vault_path_);
    vault.create("password");
    vault.lock();

    EXPECT_THROW(vault.notes_subkey(), std::runtime_error);
}

// ===================================================================
// Test 8: Subkey consistent across unlocks
// ===================================================================
TEST_F(VaultServiceTest, SubkeyConsistentAcrossUnlocks) {
    VaultService vault(vault_path_);
    vault.create("password");

    // Capture subkey data
    const auto& key1 = vault.notes_subkey();
    std::vector<uint8_t> key1_copy(key1.data(), key1.data() + key1.size());

    // Lock and unlock
    vault.lock();
    vault.unlock("password");

    // Subkey should be identical (deterministic derivation)
    const auto& key2 = vault.notes_subkey();
    ASSERT_EQ(key1_copy.size(), key2.size());
    EXPECT_EQ(0, sodium_memcmp(key1_copy.data(), key2.data(), key2.size()));
}

// ===================================================================
// Test 9: State transitions
// ===================================================================
TEST_F(VaultServiceTest, StateTransitions) {
    VaultService vault(vault_path_);

    // Initial: kNoVault
    EXPECT_EQ(VaultState::kNoVault, vault.state());

    // Create → kUnlocked
    vault.create("password");
    EXPECT_EQ(VaultState::kUnlocked, vault.state());

    // Lock → kLocked
    vault.lock();
    EXPECT_EQ(VaultState::kLocked, vault.state());

    // Unlock → kUnlocked
    vault.unlock("password");
    EXPECT_EQ(VaultState::kUnlocked, vault.state());
}

// ===================================================================
// Test 10: Persistence across VaultService instances
// ===================================================================
TEST_F(VaultServiceTest, PersistenceAcrossInstances) {
    // Create vault with first instance
    {
        VaultService vault(vault_path_);
        vault.create("my_password");
    }

    // Unlock with new instance
    {
        VaultService vault(vault_path_);
        EXPECT_EQ(VaultState::kLocked, vault.state());
        EXPECT_TRUE(vault.unlock("my_password"));
        EXPECT_TRUE(vault.is_unlocked());
    }
}

// ===================================================================
// Test 11: Empty password handling
// ===================================================================
TEST_F(VaultServiceTest, EmptyPasswordHandling) {
    VaultService vault(vault_path_);

    // Empty password should work (password strength is user's responsibility)
    EXPECT_TRUE(vault.create(""));
    vault.lock();
    EXPECT_TRUE(vault.unlock(""));

    // But wrong password should still fail
    vault.lock();
    EXPECT_FALSE(vault.unlock("notempty"));
}

// ===================================================================
// Test 12: Unicode password handling
// ===================================================================
TEST_F(VaultServiceTest, UnicodePasswordHandling) {
    std::string unicode_password = "\xd0\xbf\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c""123\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";

    VaultService vault(vault_path_);
    EXPECT_TRUE(vault.create(unicode_password));

    vault.lock();
    EXPECT_TRUE(vault.unlock(unicode_password));

    // Wrong unicode password should fail
    vault.lock();
    std::string wrong_password = "\xd0\xb4\xd1\x80\xd1\x83\xd0\xb3\xd0\xbe\xd0\xb9\xd0\xbf\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c";
    EXPECT_FALSE(vault.unlock(wrong_password));
}

// ===================================================================
// Test 13: Settings subkey accessible when unlocked
// ===================================================================
TEST_F(VaultServiceTest, SettingsSubkeyAccessible) {
    VaultService vault(vault_path_);
    vault.create("password");

    EXPECT_NO_THROW(vault.settings_subkey());

    vault.lock();
    EXPECT_THROW(vault.settings_subkey(), std::runtime_error);
}

// ===================================================================
// Test 14: Save and load settings round-trip
// ===================================================================
TEST_F(VaultServiceTest, SettingsSaveLoadRoundTrip) {
    VaultService vault(vault_path_);
    vault.create("password");

    std::string json = R"({"auto_lock_minutes":10,"clipboard_clear_enabled":false,"clipboard_clear_seconds":60})";
    vault.save_settings(json);

    std::string loaded = vault.load_settings();
    EXPECT_EQ(json, loaded);
}

// ===================================================================
// Test 15: Settings persist across lock/unlock cycles
// ===================================================================
TEST_F(VaultServiceTest, SettingsPersistAcrossUnlock) {
    std::string json = R"({"auto_lock_minutes":15,"clipboard_clear_enabled":true,"clipboard_clear_seconds":45})";

    {
        VaultService vault(vault_path_);
        vault.create("password");
        vault.save_settings(json);
    }

    // Reopen and unlock
    VaultService vault(vault_path_);
    vault.unlock("password");

    std::string loaded = vault.load_settings();
    EXPECT_EQ(json, loaded);
}

// ===================================================================
// Test 16: Load settings returns empty when none stored
// ===================================================================
TEST_F(VaultServiceTest, LoadSettingsEmptyWhenNoneStored) {
    VaultService vault(vault_path_);
    vault.create("password");

    std::string loaded = vault.load_settings();
    EXPECT_TRUE(loaded.empty());
}

// ===================================================================
// Test 17: Save settings throws when locked
// ===================================================================
TEST_F(VaultServiceTest, SaveSettingsThrowsWhenLocked) {
    VaultService vault(vault_path_);
    vault.create("password");
    vault.lock();

    EXPECT_THROW(vault.save_settings("{}"), std::runtime_error);
}

// ===================================================================
// Test 18: Settings overwritten on re-save
// ===================================================================
TEST_F(VaultServiceTest, SettingsOverwrittenOnResave) {
    VaultService vault(vault_path_);
    vault.create("password");

    vault.save_settings(R"({"auto_lock_minutes":5})");
    vault.save_settings(R"({"auto_lock_minutes":20})");

    std::string loaded = vault.load_settings();
    EXPECT_EQ(R"({"auto_lock_minutes":20})", loaded);
}
