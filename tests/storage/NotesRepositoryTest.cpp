#include <gtest/gtest.h>
#include "bastionx/storage/NotesRepository.h"
#include "bastionx/vault/VaultService.h"
#include <sodium.h>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace bastionx::storage;
using namespace bastionx::vault;
using namespace bastionx::crypto;
namespace fs = std::filesystem;

/**
 * @brief Test fixture for NotesRepository tests
 *
 * Creates a vault with VaultService, then tests NotesRepository against it.
 */
class NotesRepositoryTest : public ::testing::Test {
protected:
    std::string vault_path_;
    std::string temp_dir_;
    std::unique_ptr<VaultService> vault_;
    std::unique_ptr<NotesRepository> repo_;

    void SetUp() override {
        // Create unique temp directory
        unsigned char buf[8];
        randombytes_buf(buf, sizeof(buf));
        std::string suffix;
        for (auto b : buf) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", b);
            suffix += hex;
        }

        temp_dir_ = (fs::temp_directory_path() / ("bastionx_notes_test_" + suffix)).string();
        fs::create_directories(temp_dir_);
        vault_path_ = (fs::path(temp_dir_) / "vault.db").string();

        // Create and unlock vault
        vault_ = std::make_unique<VaultService>(vault_path_);
        vault_->create("test_password");

        // Open repository
        repo_ = std::make_unique<NotesRepository>(vault_path_);
    }

    void TearDown() override {
        repo_.reset();
        vault_.reset();
        std::error_code ec;
        fs::remove_all(temp_dir_, ec);
    }

    const SecureKey& subkey() const {
        return vault_->notes_subkey();
    }

    static Note make_note(const std::string& title, const std::string& body,
                          const std::vector<std::string>& tags = {}) {
        Note n;
        n.title = title;
        n.body = body;
        n.tags = tags;
        return n;
    }
};

// ===================================================================
// Test 1: Create note
// ===================================================================
TEST_F(NotesRepositoryTest, CreateNote) {
    auto note = make_note("Test Title", "Test Body");
    int64_t id = repo_->create_note(note, subkey());

    EXPECT_GT(id, 0);
}

// ===================================================================
// Test 2: Read note by ID (round-trip)
// ===================================================================
TEST_F(NotesRepositoryTest, ReadNoteById) {
    auto note = make_note("My Title", "My Body", {"tag1", "tag2"});
    int64_t id = repo_->create_note(note, subkey());

    auto read = repo_->read_note(id, subkey());
    ASSERT_TRUE(read.has_value());
    EXPECT_EQ(id, read->id);
    EXPECT_EQ("My Title", read->title);
    EXPECT_EQ("My Body", read->body);
    ASSERT_EQ(2, read->tags.size());
    EXPECT_EQ("tag1", read->tags[0]);
    EXPECT_EQ("tag2", read->tags[1]);
    EXPECT_GT(read->created_at, 0);
    EXPECT_GT(read->updated_at, 0);
}

// ===================================================================
// Test 3: Read nonexistent note
// ===================================================================
TEST_F(NotesRepositoryTest, ReadNonexistentNote) {
    auto read = repo_->read_note(99999, subkey());
    EXPECT_FALSE(read.has_value());
}

// ===================================================================
// Test 4: List notes
// ===================================================================
TEST_F(NotesRepositoryTest, ListNotes) {
    repo_->create_note(make_note("Note 1", "Body 1"), subkey());
    repo_->create_note(make_note("Note 2", "Body 2"), subkey());
    repo_->create_note(make_note("Note 3", "Body 3"), subkey());

    auto summaries = repo_->list_notes(subkey());
    EXPECT_EQ(3, summaries.size());

    // Verify titles are present
    std::vector<std::string> titles;
    for (const auto& s : summaries) {
        titles.push_back(s.title);
    }
    EXPECT_NE(std::find(titles.begin(), titles.end(), "Note 1"), titles.end());
    EXPECT_NE(std::find(titles.begin(), titles.end(), "Note 2"), titles.end());
    EXPECT_NE(std::find(titles.begin(), titles.end(), "Note 3"), titles.end());
}

// ===================================================================
// Test 5: List notes on empty DB
// ===================================================================
TEST_F(NotesRepositoryTest, ListNotesEmpty) {
    auto summaries = repo_->list_notes(subkey());
    EXPECT_TRUE(summaries.empty());
}

// ===================================================================
// Test 6: List notes order (most recent first)
// ===================================================================
TEST_F(NotesRepositoryTest, ListNotesOrder) {
    int64_t id1 = repo_->create_note(make_note("First", ""), subkey());

    // Small delay to ensure different timestamps
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    int64_t id2 = repo_->create_note(make_note("Second", ""), subkey());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    int64_t id3 = repo_->create_note(make_note("Third", ""), subkey());

    auto summaries = repo_->list_notes(subkey());
    ASSERT_EQ(3, summaries.size());

    // Most recent first (Third was created last)
    EXPECT_EQ(id3, summaries[0].id);
    EXPECT_EQ(id2, summaries[1].id);
    EXPECT_EQ(id1, summaries[2].id);
}

// ===================================================================
// Test 7: Update note
// ===================================================================
TEST_F(NotesRepositoryTest, UpdateNote) {
    auto note = make_note("Original", "Original body");
    int64_t id = repo_->create_note(note, subkey());

    // Read to get timestamps
    auto original = repo_->read_note(id, subkey());
    ASSERT_TRUE(original.has_value());

    // Update
    original->title = "Updated";
    original->body = "Updated body";
    original->tags = {"new_tag"};

    bool updated = repo_->update_note(*original, subkey());
    EXPECT_TRUE(updated);

    // Read back
    auto result = repo_->read_note(id, subkey());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("Updated", result->title);
    EXPECT_EQ("Updated body", result->body);
    ASSERT_EQ(1, result->tags.size());
    EXPECT_EQ("new_tag", result->tags[0]);
}

// ===================================================================
// Test 8: Update nonexistent note
// ===================================================================
TEST_F(NotesRepositoryTest, UpdateNonexistentNote) {
    Note note;
    note.id = 99999;
    note.title = "Ghost";
    note.body = "Ghost body";

    EXPECT_FALSE(repo_->update_note(note, subkey()));
}

// ===================================================================
// Test 9: Delete note
// ===================================================================
TEST_F(NotesRepositoryTest, DeleteNote) {
    int64_t id = repo_->create_note(make_note("To Delete", ""), subkey());
    EXPECT_TRUE(repo_->delete_note(id));

    // Verify deleted
    EXPECT_FALSE(repo_->read_note(id, subkey()).has_value());
}

// ===================================================================
// Test 10: Delete nonexistent note
// ===================================================================
TEST_F(NotesRepositoryTest, DeleteNonexistentNote) {
    EXPECT_FALSE(repo_->delete_note(99999));
}

// ===================================================================
// Test 11: Note with all fields survives round-trip
// ===================================================================
TEST_F(NotesRepositoryTest, NoteWithAllFields) {
    auto note = make_note(
        "Full Note",
        "This is the body with multiple lines.\nLine 2.\nLine 3.",
        {"personal", "diary", "important"}
    );
    int64_t id = repo_->create_note(note, subkey());

    auto read = repo_->read_note(id, subkey());
    ASSERT_TRUE(read.has_value());
    EXPECT_EQ("Full Note", read->title);
    EXPECT_EQ("This is the body with multiple lines.\nLine 2.\nLine 3.", read->body);
    ASSERT_EQ(3, read->tags.size());
    EXPECT_EQ("personal", read->tags[0]);
    EXPECT_EQ("diary", read->tags[1]);
    EXPECT_EQ("important", read->tags[2]);
}

// ===================================================================
// Test 12: Note with empty fields
// ===================================================================
TEST_F(NotesRepositoryTest, NoteWithEmptyFields) {
    auto note = make_note("", "", {});
    int64_t id = repo_->create_note(note, subkey());

    auto read = repo_->read_note(id, subkey());
    ASSERT_TRUE(read.has_value());
    EXPECT_EQ("", read->title);
    EXPECT_EQ("", read->body);
    EXPECT_TRUE(read->tags.empty());
}

// ===================================================================
// Test 13: Note with Unicode content
// ===================================================================
TEST_F(NotesRepositoryTest, NoteWithUnicodeContent) {
    auto note = make_note(
        u8"日本語タイトル",
        u8"Содержимое на русском языке 🔐",
        {u8"тег", u8"日本語"}
    );
    int64_t id = repo_->create_note(note, subkey());

    auto read = repo_->read_note(id, subkey());
    ASSERT_TRUE(read.has_value());
    EXPECT_EQ(u8"日本語タイトル", read->title);
    EXPECT_EQ(u8"Содержимое на русском языке 🔐", read->body);
    ASSERT_EQ(2, read->tags.size());
    EXPECT_EQ(u8"тег", read->tags[0]);
    EXPECT_EQ(u8"日本語", read->tags[1]);
}

// ===================================================================
// Test 14: Wrong key cannot decrypt
// ===================================================================
TEST_F(NotesRepositoryTest, WrongKeyCannotDecrypt) {
    int64_t id = repo_->create_note(make_note("Secret", "Secret body"), subkey());

    // Derive a different key
    auto different_master = CryptoService::derive_master_key("different_password");
    auto different_subkey = CryptoService::derive_subkey(
        different_master.master_key, CryptoService::SUBKEY_NOTES);

    // Try reading with wrong key
    auto read = repo_->read_note(id, different_subkey);
    EXPECT_FALSE(read.has_value());
}

// ===================================================================
// Test 15: Fresh nonce on update
// ===================================================================
TEST_F(NotesRepositoryTest, FreshNonceOnUpdate) {
    auto note = make_note("Original", "Body");
    int64_t id = repo_->create_note(note, subkey());

    // Read original nonce from DB directly
    sqlite3* db = nullptr;
    sqlite3_open(vault_path_.c_str(), &db);

    auto get_nonce = [&](int64_t note_id) -> std::vector<uint8_t> {
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, "SELECT nonce FROM notes WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int64(stmt, 1, note_id);
        sqlite3_step(stmt);
        const void* blob = sqlite3_column_blob(stmt, 0);
        int size = sqlite3_column_bytes(stmt, 0);
        std::vector<uint8_t> nonce(static_cast<const uint8_t*>(blob),
                                   static_cast<const uint8_t*>(blob) + size);
        sqlite3_finalize(stmt);
        return nonce;
    };

    auto nonce_before = get_nonce(id);

    // Update note
    auto read = repo_->read_note(id, subkey());
    read->body = "Updated body";
    repo_->update_note(*read, subkey());

    auto nonce_after = get_nonce(id);

    sqlite3_close(db);

    // Nonces should be different (fresh random nonce per encryption)
    EXPECT_NE(nonce_before, nonce_after);
}
