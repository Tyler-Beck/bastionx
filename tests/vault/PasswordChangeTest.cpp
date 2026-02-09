#include <gtest/gtest.h>
#include "bastionx/vault/VaultService.h"
#include "bastionx/storage/NotesRepository.h"
#include <sodium.h>
#include <filesystem>
#include <string>

using namespace bastionx::vault;
using namespace bastionx::crypto;
using namespace bastionx::storage;
namespace fs = std::filesystem;

class PasswordChangeTest : public ::testing::Test {
protected:
    std::string vault_path_;
    std::string temp_dir_;

    void SetUp() override {
        temp_dir_ = (fs::temp_directory_path() / ("bastionx_pwchg_" + random_suffix())).string();
        fs::create_directories(temp_dir_);
        vault_path_ = (fs::path(temp_dir_) / "vault.db").string();
    }

    void TearDown() override {
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
// Test 1: Basic password change works
// ===================================================================
TEST_F(PasswordChangeTest, BasicPasswordChange) {
    VaultService vault(vault_path_);
    vault.create("old_password");

    bool changed = vault.change_password("old_password", "new_password");
    EXPECT_TRUE(changed);

    // Lock and verify new password works
    vault.lock();
    EXPECT_TRUE(vault.unlock("new_password"));
}

// ===================================================================
// Test 2: Old password fails after change
// ===================================================================
TEST_F(PasswordChangeTest, OldPasswordFailsAfterChange) {
    VaultService vault(vault_path_);
    vault.create("old_password");
    vault.change_password("old_password", "new_password");

    vault.lock();
    EXPECT_FALSE(vault.unlock("old_password"));
}

// ===================================================================
// Test 3: Wrong current password rejected
// ===================================================================
TEST_F(PasswordChangeTest, WrongCurrentPasswordRejected) {
    VaultService vault(vault_path_);
    vault.create("my_password");

    bool changed = vault.change_password("wrong_password", "new_password");
    EXPECT_FALSE(changed);

    // Original password should still work
    vault.lock();
    EXPECT_TRUE(vault.unlock("my_password"));
}

// ===================================================================
// Test 4: Notes survive password change
// ===================================================================
TEST_F(PasswordChangeTest, NotesSurvivePasswordChange) {
    VaultService vault(vault_path_);
    vault.create("old_pw");

    // Create some notes
    {
        NotesRepository repo(vault_path_, &vault.db_subkey());
        Note n1;
        n1.title = "Secret Note";
        n1.body = "This is sensitive data";
        repo.create_note(n1, vault.notes_subkey());

        Note n2;
        n2.title = "Another Note";
        n2.body = "More sensitive info";
        repo.create_note(n2, vault.notes_subkey());
    }

    // Change password
    vault.change_password("old_pw", "new_pw");

    // Verify notes are readable with new key
    {
        NotesRepository repo(vault_path_, &vault.db_subkey());
        auto summaries = repo.list_notes(vault.notes_subkey());
        ASSERT_EQ(summaries.size(), 2u);

        // Read each note
        for (const auto& s : summaries) {
            auto note = repo.read_note(s.id, vault.notes_subkey());
            ASSERT_TRUE(note.has_value());
        }
    }

    // Lock, unlock with new password, verify notes again
    vault.lock();
    vault.unlock("new_pw");

    {
        NotesRepository repo(vault_path_, &vault.db_subkey());
        auto summaries = repo.list_notes(vault.notes_subkey());
        ASSERT_EQ(summaries.size(), 2u);

        auto note = repo.read_note(summaries[0].id, vault.notes_subkey());
        ASSERT_TRUE(note.has_value());
        // One of them should have this title (order is by updated_at DESC)
        bool found_secret = false;
        bool found_another = false;
        for (const auto& s : summaries) {
            auto n = repo.read_note(s.id, vault.notes_subkey());
            if (n->title == "Secret Note") found_secret = true;
            if (n->title == "Another Note") found_another = true;
        }
        EXPECT_TRUE(found_secret);
        EXPECT_TRUE(found_another);
    }
}

// ===================================================================
// Test 5: Settings survive password change
// ===================================================================
TEST_F(PasswordChangeTest, SettingsSurvivePasswordChange) {
    VaultService vault(vault_path_);
    vault.create("old_pw");

    std::string settings_json = R"({"auto_lock_minutes":10,"clipboard_clear_enabled":false,"clipboard_clear_seconds":60})";
    vault.save_settings(settings_json);

    vault.change_password("old_pw", "new_pw");

    std::string loaded = vault.load_settings();
    EXPECT_EQ(settings_json, loaded);

    // Also verify after lock/unlock cycle
    vault.lock();
    vault.unlock("new_pw");
    loaded = vault.load_settings();
    EXPECT_EQ(settings_json, loaded);
}

// ===================================================================
// Test 6: Change password on empty vault (no notes)
// ===================================================================
TEST_F(PasswordChangeTest, ChangePasswordEmptyVault) {
    VaultService vault(vault_path_);
    vault.create("old_pw");

    bool changed = vault.change_password("old_pw", "new_pw");
    EXPECT_TRUE(changed);

    vault.lock();
    EXPECT_TRUE(vault.unlock("new_pw"));
}

// ===================================================================
// Test 7: Change password throws when locked
// ===================================================================
TEST_F(PasswordChangeTest, ThrowsWhenLocked) {
    VaultService vault(vault_path_);
    vault.create("password");
    vault.lock();

    EXPECT_THROW(vault.change_password("password", "new_pw"), std::runtime_error);
}
