#include "bastionx/ui/FormattingToolbar.h"
#include <QStyle>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextListFormat>
#include <QTextList>
#include <QShortcut>
#include <QFont>

namespace bastionx {
namespace ui {

FormattingToolbar::FormattingToolbar(QTextEdit* editor, QWidget* parent)
    : QWidget(parent), editor_(editor)
{
    setObjectName("formattingToolbar");
    setFixedHeight(32);
    setupUi();

    connect(editor_, &QTextEdit::cursorPositionChanged,
            this, &FormattingToolbar::updateButtonStates);

    // Keyboard shortcuts
    auto* boldShortcut = new QShortcut(QKeySequence("Ctrl+B"), editor_);
    connect(boldShortcut, &QShortcut::activated, this, &FormattingToolbar::onBold);

    auto* italicShortcut = new QShortcut(QKeySequence("Ctrl+I"), editor_);
    connect(italicShortcut, &QShortcut::activated, this, &FormattingToolbar::onItalic);

    auto* underlineShortcut = new QShortcut(QKeySequence("Ctrl+U"), editor_);
    connect(underlineShortcut, &QShortcut::activated, this, &FormattingToolbar::onUnderline);
}

void FormattingToolbar::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(1);

    // Text formatting group
    bold_btn_ = makeButton("B", "Bold (Ctrl+B)");
    italic_btn_ = makeButton("I", "Italic (Ctrl+I)");
    underline_btn_ = makeButton("U", "Underline (Ctrl+U)");
    strike_btn_ = makeButton("S", "Strikethrough");

    layout->addWidget(bold_btn_);
    layout->addWidget(italic_btn_);
    layout->addWidget(underline_btn_);
    layout->addWidget(strike_btn_);

    layout->addWidget(makeSeparator());

    // Heading group
    h1_btn_ = makeButton("H1", "Heading 1");
    h2_btn_ = makeButton("H2", "Heading 2");
    h3_btn_ = makeButton("H3", "Heading 3");

    layout->addWidget(h1_btn_);
    layout->addWidget(h2_btn_);
    layout->addWidget(h3_btn_);

    layout->addWidget(makeSeparator());

    // List group
    bullet_btn_ = makeButton("-", "Bullet List");
    numbered_btn_ = makeButton("1.", "Numbered List");

    layout->addWidget(bullet_btn_);
    layout->addWidget(numbered_btn_);

    layout->addWidget(makeSeparator());

    // Block group
    quote_btn_ = makeButton(">", "Blockquote");
    code_btn_ = makeButton("<>", "Code Block");
    hr_btn_ = makeButton("--", "Horizontal Rule");

    layout->addWidget(quote_btn_);
    layout->addWidget(code_btn_);
    layout->addWidget(hr_btn_);

    layout->addStretch();

    // Connect buttons
    connect(bold_btn_, &QPushButton::clicked, this, &FormattingToolbar::onBold);
    connect(italic_btn_, &QPushButton::clicked, this, &FormattingToolbar::onItalic);
    connect(underline_btn_, &QPushButton::clicked, this, &FormattingToolbar::onUnderline);
    connect(strike_btn_, &QPushButton::clicked, this, &FormattingToolbar::onStrikethrough);
    connect(h1_btn_, &QPushButton::clicked, this, [this] { onHeading(1); });
    connect(h2_btn_, &QPushButton::clicked, this, [this] { onHeading(2); });
    connect(h3_btn_, &QPushButton::clicked, this, [this] { onHeading(3); });
    connect(bullet_btn_, &QPushButton::clicked, this, &FormattingToolbar::onBulletList);
    connect(numbered_btn_, &QPushButton::clicked, this, &FormattingToolbar::onNumberedList);
    connect(quote_btn_, &QPushButton::clicked, this, &FormattingToolbar::onBlockquote);
    connect(code_btn_, &QPushButton::clicked, this, &FormattingToolbar::onCodeBlock);
    connect(hr_btn_, &QPushButton::clicked, this, &FormattingToolbar::onHorizontalRule);
}

QPushButton* FormattingToolbar::makeButton(const QString& text, const QString& tooltip) {
    auto* btn = new QPushButton(text, this);
    btn->setObjectName("formatButton");
    btn->setToolTip(tooltip);
    btn->setFlat(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedSize(28, 24);
    return btn;
}

QWidget* FormattingToolbar::makeSeparator() {
    auto* sep = new QWidget(this);
    sep->setFixedSize(1, 20);
    sep->setStyleSheet("background-color: #3a3a3a;");
    return sep;
}

void FormattingToolbar::setButtonActive(QPushButton* btn, bool active) {
    btn->setObjectName(active ? "formatButtonActive" : "formatButton");
    btn->style()->unpolish(btn);
    btn->style()->polish(btn);
}

void FormattingToolbar::onBold() {
    auto cursor = editor_->textCursor();
    QTextCharFormat fmt;
    bool wasBold = cursor.charFormat().fontWeight() == QFont::Bold;
    fmt.setFontWeight(wasBold ? QFont::Normal : QFont::Bold);
    cursor.mergeCharFormat(fmt);
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onItalic() {
    auto cursor = editor_->textCursor();
    QTextCharFormat fmt;
    fmt.setFontItalic(!cursor.charFormat().fontItalic());
    cursor.mergeCharFormat(fmt);
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onUnderline() {
    auto cursor = editor_->textCursor();
    QTextCharFormat fmt;
    fmt.setFontUnderline(!cursor.charFormat().fontUnderline());
    cursor.mergeCharFormat(fmt);
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onStrikethrough() {
    auto cursor = editor_->textCursor();
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(!cursor.charFormat().fontStrikeOut());
    cursor.mergeCharFormat(fmt);
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onHeading(int level) {
    auto cursor = editor_->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();
    int currentLevel = blockFmt.headingLevel();

    // Toggle: if same level, revert to normal paragraph
    int newLevel = (currentLevel == level) ? 0 : level;

    blockFmt.setHeadingLevel(newLevel);
    cursor.setBlockFormat(blockFmt);

    // Set appropriate font size for heading
    QTextCharFormat charFmt;
    if (newLevel == 1) {
        charFmt.setFontPointSize(24);
        charFmt.setFontWeight(QFont::Bold);
    } else if (newLevel == 2) {
        charFmt.setFontPointSize(20);
        charFmt.setFontWeight(QFont::Bold);
    } else if (newLevel == 3) {
        charFmt.setFontPointSize(16);
        charFmt.setFontWeight(QFont::Bold);
    } else {
        charFmt.setFontPointSize(14);
        charFmt.setFontWeight(QFont::Normal);
    }

    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.mergeCharFormat(charFmt);
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onBulletList() {
    auto cursor = editor_->textCursor();
    QTextList* currentList = cursor.currentList();

    if (currentList && currentList->format().style() == QTextListFormat::ListDisc) {
        // Remove from list
        QTextBlockFormat blockFmt;
        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);
    } else {
        QTextListFormat listFmt;
        listFmt.setStyle(QTextListFormat::ListDisc);
        cursor.createList(listFmt);
    }
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onNumberedList() {
    auto cursor = editor_->textCursor();
    QTextList* currentList = cursor.currentList();

    if (currentList && currentList->format().style() == QTextListFormat::ListDecimal) {
        // Remove from list
        QTextBlockFormat blockFmt;
        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);
    } else {
        QTextListFormat listFmt;
        listFmt.setStyle(QTextListFormat::ListDecimal);
        cursor.createList(listFmt);
    }
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onBlockquote() {
    auto cursor = editor_->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();

    int quoteLevel = blockFmt.property(QTextFormat::BlockQuoteLevel).toInt();
    blockFmt.setProperty(QTextFormat::BlockQuoteLevel, quoteLevel > 0 ? 0 : 1);
    cursor.setBlockFormat(blockFmt);
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onCodeBlock() {
    auto cursor = editor_->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();

    bool isCode = blockFmt.property(QTextFormat::BlockCodeFence).toBool();

    if (isCode) {
        // Remove code block
        blockFmt.clearProperty(QTextFormat::BlockCodeFence);
        blockFmt.clearProperty(QTextFormat::BlockCodeLanguage);
        cursor.setBlockFormat(blockFmt);

        QTextCharFormat charFmt;
        charFmt.setFontFamilies({"Fira Mono", "Consolas", "JetBrains Mono"});
        charFmt.setFontPointSize(14);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.mergeCharFormat(charFmt);
    } else {
        // Set code block
        blockFmt.setProperty(QTextFormat::BlockCodeFence, true);
        cursor.setBlockFormat(blockFmt);

        QTextCharFormat charFmt;
        charFmt.setFontFamilies({"Fira Mono", "Consolas", "monospace"});
        charFmt.setFontPointSize(13);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.mergeCharFormat(charFmt);
    }
    editor_->setTextCursor(cursor);
    updateButtonStates();
}

void FormattingToolbar::onHorizontalRule() {
    auto cursor = editor_->textCursor();
    cursor.insertHtml("<hr/>");
    editor_->setTextCursor(cursor);
}

void FormattingToolbar::updateButtonStates() {
    auto cursor = editor_->textCursor();
    QTextCharFormat charFmt = cursor.charFormat();
    QTextBlockFormat blockFmt = cursor.blockFormat();

    // Character formats
    setButtonActive(bold_btn_, charFmt.fontWeight() == QFont::Bold);
    setButtonActive(italic_btn_, charFmt.fontItalic());
    setButtonActive(underline_btn_, charFmt.fontUnderline());
    setButtonActive(strike_btn_, charFmt.fontStrikeOut());

    // Heading level
    int headingLevel = blockFmt.headingLevel();
    setButtonActive(h1_btn_, headingLevel == 1);
    setButtonActive(h2_btn_, headingLevel == 2);
    setButtonActive(h3_btn_, headingLevel == 3);

    // Lists
    QTextList* list = cursor.currentList();
    bool isBullet = list && list->format().style() == QTextListFormat::ListDisc;
    bool isNumbered = list && list->format().style() == QTextListFormat::ListDecimal;
    setButtonActive(bullet_btn_, isBullet);
    setButtonActive(numbered_btn_, isNumbered);

    // Block formats
    int quoteLevel = blockFmt.property(QTextFormat::BlockQuoteLevel).toInt();
    setButtonActive(quote_btn_, quoteLevel > 0);

    bool isCode = blockFmt.property(QTextFormat::BlockCodeFence).toBool();
    setButtonActive(code_btn_, isCode);
}

}  // namespace ui
}  // namespace bastionx
