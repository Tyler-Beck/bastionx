#include <gtest/gtest.h>
#include <QApplication>
#include <QTextDocument>
#include <QString>

class MarkdownRoundTripTest : public ::testing::Test {
protected:
    // Set markdown on a QTextDocument and read it back
    std::string roundTrip(const std::string& input) {
        QTextDocument doc;
        doc.setMarkdown(QString::fromStdString(input));
        return doc.toMarkdown().toStdString();
    }

    // Check that the round-tripped output contains the expected substring
    bool roundTripContains(const std::string& input, const std::string& expected) {
        std::string output = roundTrip(input);
        return output.find(expected) != std::string::npos;
    }
};

TEST_F(MarkdownRoundTripTest, PlainTextSurvives) {
    std::string output = roundTrip("Hello world");
    EXPECT_NE(output.find("Hello world"), std::string::npos);
}

TEST_F(MarkdownRoundTripTest, BoldTextSurvives) {
    EXPECT_TRUE(roundTripContains("**bold text**", "**bold text**"));
}

TEST_F(MarkdownRoundTripTest, ItalicTextSurvives) {
    EXPECT_TRUE(roundTripContains("*italic text*", "*italic text*"));
}

TEST_F(MarkdownRoundTripTest, Heading1Survives) {
    EXPECT_TRUE(roundTripContains("# Heading 1", "# Heading 1"));
}

TEST_F(MarkdownRoundTripTest, Heading2Survives) {
    EXPECT_TRUE(roundTripContains("## Heading 2", "## Heading 2"));
}

TEST_F(MarkdownRoundTripTest, Heading3Survives) {
    EXPECT_TRUE(roundTripContains("### Heading 3", "### Heading 3"));
}

TEST_F(MarkdownRoundTripTest, BulletListSurvives) {
    std::string input = "- item one\n- item two\n- item three\n";
    std::string output = roundTrip(input);
    EXPECT_NE(output.find("item one"), std::string::npos);
    EXPECT_NE(output.find("item two"), std::string::npos);
    EXPECT_NE(output.find("item three"), std::string::npos);
}

TEST_F(MarkdownRoundTripTest, NumberedListSurvives) {
    std::string input = "1. first\n2. second\n3. third\n";
    std::string output = roundTrip(input);
    EXPECT_NE(output.find("first"), std::string::npos);
    EXPECT_NE(output.find("second"), std::string::npos);
    EXPECT_NE(output.find("third"), std::string::npos);
}

TEST_F(MarkdownRoundTripTest, MixedFormattingSurvives) {
    std::string input = "# Title\n\nSome **bold** and *italic* text.\n";
    std::string output = roundTrip(input);
    EXPECT_NE(output.find("# Title"), std::string::npos);
    EXPECT_NE(output.find("**bold**"), std::string::npos);
    EXPECT_NE(output.find("*italic*"), std::string::npos);
}

TEST_F(MarkdownRoundTripTest, EmptyDocumentHandled) {
    std::string output = roundTrip("");
    // Should not crash, output may be empty or whitespace
    EXPECT_TRUE(output.empty() || output.find_first_not_of(" \n\r\t") == std::string::npos);
}

TEST_F(MarkdownRoundTripTest, BlockquoteSurvives) {
    EXPECT_TRUE(roundTripContains("> quoted text", "> quoted text"));
}

TEST_F(MarkdownRoundTripTest, CodeBlockSurvives) {
    std::string input = "```\ncode here\n```\n";
    std::string output = roundTrip(input);
    EXPECT_NE(output.find("code here"), std::string::npos);
}
