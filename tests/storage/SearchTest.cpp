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

class SearchTest : public ::testing::Test {
protected:
    std::string temp_dir_;
    std::string vault_path_;
    std::unique_ptr<VaultService> vault_;
    std::unique_ptr<NotesRepository> repo_;

    void SetUp() override {
        unsigned char buf[8];
        randombytes_buf(buf, sizeof(buf));
        std::string suffix;
        for (auto b : buf) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", b);
            suffix += hex;
        }

        temp_dir_ = (fs::temp_directory_path() / ("bastionx_search_test_" + suffix)).string();
        fs::create_directories(temp_dir_);
        vault_path_ = (fs::path(temp_dir_) / "vault.db").string();

        vault_ = std::make_unique<VaultService>(vault_path_);
        vault_->create("test_password");

        repo_ = std::make_unique<NotesRepository>(vault_path_, &vault_->db_subkey());
    }

    void TearDown() override {
        repo_.reset();
        vault_.reset();
        std::error_code ec;
        fs::remove_all(temp_dir_, ec);
    }

    const SecureKey& subkey() const { return vault_->notes_subkey(); }

    static Note make_note(const std::string& title, const std::string& body,
                          const std::vector<std::string>& tags = {}) {
        Note n;
        n.title = title;
        n.body = body;
        n.tags = tags;
        return n;
    }
};

TEST_F(SearchTest, SearchByTitleCaseInsensitive) {
    repo_->create_note(make_note("Meeting Notes", "discussed budgets"), subkey());
    repo_->create_note(make_note("Shopping List", "milk eggs bread"), subkey());

    auto results = repo_->search_notes(subkey(), "meeting");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].title, "Meeting Notes");
}

TEST_F(SearchTest, SearchByBodySubstring) {
    repo_->create_note(make_note("Note A", "the quick brown fox jumps"), subkey());
    repo_->create_note(make_note("Note B", "lazy dog sleeping"), subkey());

    auto results = repo_->search_notes(subkey(), "brown fox");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].title, "Note A");
}

TEST_F(SearchTest, SearchByTag) {
    repo_->create_note(make_note("Work", "some content", {"project", "urgent"}), subkey());
    repo_->create_note(make_note("Personal", "other content", {"home"}), subkey());

    auto results = repo_->search_notes(subkey(), "urgent");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].title, "Work");
}

TEST_F(SearchTest, EmptyQueryReturnsEmpty) {
    repo_->create_note(make_note("Test", "content"), subkey());
    auto results = repo_->search_notes(subkey(), "");
    EXPECT_TRUE(results.empty());
}

TEST_F(SearchTest, SingleCharQueryReturnsEmpty) {
    repo_->create_note(make_note("Test", "content"), subkey());
    auto results = repo_->search_notes(subkey(), "x");
    EXPECT_TRUE(results.empty());
}

TEST_F(SearchTest, NoMatchReturnsEmpty) {
    repo_->create_note(make_note("Hello", "world"), subkey());
    auto results = repo_->search_notes(subkey(), "zzzzz");
    EXPECT_TRUE(results.empty());
}

TEST_F(SearchTest, MultipleMatchesSortedByUpdatedAt) {
    repo_->create_note(make_note("Alpha notes", "alpha content"), subkey());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    repo_->create_note(make_note("Beta notes", "more alpha here"), subkey());

    auto results = repo_->search_notes(subkey(), "alpha");
    ASSERT_EQ(results.size(), 2);
    // Most recently updated first
    EXPECT_EQ(results[0].title, "Beta notes");
    EXPECT_EQ(results[1].title, "Alpha notes");
}

TEST_F(SearchTest, DeletedNoteNotReturned) {
    auto id = repo_->create_note(make_note("Delete Me", "findable text"), subkey());
    repo_->delete_note(id);

    auto results = repo_->search_notes(subkey(), "findable");
    EXPECT_TRUE(results.empty());
}

TEST_F(SearchTest, BodySnippetContainsContext) {
    std::string long_body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                            "The secret keyword is hidden deep inside this long note body. "
                            "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
    repo_->create_note(make_note("Long Note", long_body), subkey());

    auto results = repo_->search_notes(subkey(), "secret keyword");
    ASSERT_EQ(results.size(), 1);
    // Preview should contain the match context, not just the first 80 chars
    EXPECT_NE(results[0].preview.find("secret keyword"), std::string::npos);
}

TEST_F(SearchTest, TagSearchCaseInsensitive) {
    repo_->create_note(make_note("Tagged", "body", {"ImportantTag"}), subkey());

    auto results = repo_->search_notes(subkey(), "importanttag");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].title, "Tagged");
}
