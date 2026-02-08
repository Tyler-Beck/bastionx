#include <gtest/gtest.h>
#include "bastionx/vault/VaultService.h"
#include "bastionx/storage/NotesRepository.h"
#include <sodium.h>
#include <filesystem>

using namespace bastionx::vault;
using namespace bastionx::storage;
using namespace bastionx::crypto;
namespace fs = std::filesystem;

/**
 * @brief Integration tests for VaultService + NotesRepository
 *
 * Tests the full vault lifecycle: create, unlock, CRUD notes, lock, re-unlock.
 */
class IntegrationTest : public ::testing::Test {
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

        temp_dir_ = (fs::temp_directory_path() / ("bastionx_integ_test_" + suffix)).string();
        fs::create_directories(temp_dir_);
        vault_path_ = (fs::path(temp_dir_) / "vault.db").string();
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(temp_dir_, ec);
    }
};

// ===================================================================
// Test 1: Full lifecycle
// ===================================================================
TEST_F(IntegrationTest, FullLifecycle) {
    // Create vault
    VaultService vault(vault_path_);
    ASSERT_TRUE(vault.create("my_password"));

    // Open repository
    NotesRepository repo(vault_path_);

    // Create notes
    Note n1;
    n1.title = "First Note";
    n1.body = "Content of first note";
    n1.tags = {"work"};
    int64_t id1 = repo.create_note(n1, vault.notes_subkey());

    Note n2;
    n2.title = "Second Note";
    n2.body = "Content of second note";
    n2.tags = {"personal"};
    int64_t id2 = repo.create_note(n2, vault.notes_subkey());

    Note n3;
    n3.title = "Third Note";
    n3.body = "Content of third note";
    int64_t id3 = repo.create_note(n3, vault.notes_subkey());

    // List notes (should be 3)
    auto summaries = repo.list_notes(vault.notes_subkey());
    EXPECT_EQ(3, summaries.size());

    // Read each note
    auto read1 = repo.read_note(id1, vault.notes_subkey());
    ASSERT_TRUE(read1.has_value());
    EXPECT_EQ("First Note", read1->title);
    EXPECT_EQ("Content of first note", read1->body);

    auto read2 = repo.read_note(id2, vault.notes_subkey());
    ASSERT_TRUE(read2.has_value());
    EXPECT_EQ("Second Note", read2->title);

    // Update note 2
    read2->title = "Updated Second Note";
    read2->body = "Updated content";
    EXPECT_TRUE(repo.update_note(*read2, vault.notes_subkey()));

    auto updated = repo.read_note(id2, vault.notes_subkey());
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ("Updated Second Note", updated->title);
    EXPECT_EQ("Updated content", updated->body);

    // Delete note 1
    EXPECT_TRUE(repo.delete_note(id1));
    EXPECT_FALSE(repo.read_note(id1, vault.notes_subkey()).has_value());

    // List should now have 2
    summaries = repo.list_notes(vault.notes_subkey());
    EXPECT_EQ(2, summaries.size());

    // Close repo before locking
    repo.close();

    // Lock vault
    vault.lock();
    EXPECT_THROW(vault.notes_subkey(), std::runtime_error);

    // Unlock vault
    EXPECT_TRUE(vault.unlock("my_password"));

    // Reopen repo and verify data persisted
    NotesRepository repo2(vault_path_);
    summaries = repo2.list_notes(vault.notes_subkey());
    EXPECT_EQ(2, summaries.size());

    auto read3 = repo2.read_note(id3, vault.notes_subkey());
    ASSERT_TRUE(read3.has_value());
    EXPECT_EQ("Third Note", read3->title);
}

// ===================================================================
// Test 2: Persistence across restarts
// ===================================================================
TEST_F(IntegrationTest, PersistenceAcrossRestarts) {
    int64_t saved_id = 0;

    // Session 1: Create vault and notes
    {
        VaultService vault(vault_path_);
        vault.create("persistent_password");

        NotesRepository repo(vault_path_);
        Note n;
        n.title = "Persistent Note";
        n.body = "This should survive a restart";
        n.tags = {"important"};
        saved_id = repo.create_note(n, vault.notes_subkey());
    }
    // Both vault and repo are destroyed here

    // Session 2: Reopen and verify
    {
        VaultService vault(vault_path_);
        EXPECT_TRUE(vault.unlock("persistent_password"));

        NotesRepository repo(vault_path_);
        auto note = repo.read_note(saved_id, vault.notes_subkey());
        ASSERT_TRUE(note.has_value());
        EXPECT_EQ("Persistent Note", note->title);
        EXPECT_EQ("This should survive a restart", note->body);
        ASSERT_EQ(1, note->tags.size());
        EXPECT_EQ("important", note->tags[0]);
    }
}

// ===================================================================
// Test 3: Wrong password cannot read notes
// ===================================================================
TEST_F(IntegrationTest, WrongPasswordCannotReadNotes) {
    // Create vault with notes
    {
        VaultService vault(vault_path_);
        vault.create("correct");

        NotesRepository repo(vault_path_);
        repo.create_note(Note{0, "Secret", "Top secret content", {}, 0, 0}, vault.notes_subkey());
    }

    // Try with wrong password
    {
        VaultService vault(vault_path_);
        EXPECT_FALSE(vault.unlock("wrong"));
        EXPECT_FALSE(vault.is_unlocked());
    }
}

// ===================================================================
// Test 4: Empty vault unlock cycle
// ===================================================================
TEST_F(IntegrationTest, EmptyVaultUnlockCycle) {
    // Create empty vault
    VaultService vault(vault_path_);
    vault.create("password");

    // Lock and unlock with no notes
    vault.lock();
    EXPECT_TRUE(vault.unlock("password"));

    // List notes (should be empty)
    NotesRepository repo(vault_path_);
    auto summaries = repo.list_notes(vault.notes_subkey());
    EXPECT_TRUE(summaries.empty());

    // Now create a note
    Note n;
    n.title = "After Unlock";
    n.body = "Created after unlock cycle";
    int64_t id = repo.create_note(n, vault.notes_subkey());
    EXPECT_GT(id, 0);

    // Close repo, lock, unlock, reopen, verify
    repo.close();
    vault.lock();
    EXPECT_TRUE(vault.unlock("password"));

    NotesRepository repo2(vault_path_);
    summaries = repo2.list_notes(vault.notes_subkey());
    ASSERT_EQ(1, summaries.size());
    EXPECT_EQ("After Unlock", summaries[0].title);
}
