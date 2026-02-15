#include "bastionx/ui/FormattingToolbar.h"
#include "bastionx/ui/UIConstants.h"
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

using namespace constants;

FormattingToolbar::FormattingToolbar(QTextEdit* editor, QWidget* parent)
    : QWidget(parent), editor_(editor)
{
    setObjectName("formattingToolbar");
    setFixedHeight(kFormattingToolbarHeight);
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
    layout->setContentsMargins(kMarginSmall, kMarginTiny, kMarginSmall, kMarginTiny);
    layout->setSpacing(kSpacingTight);

    // Text formatting group
    // Text formatting group (stronger symbols)
    bold_btn_ = makeButton("𝐁", "Bold (Ctrl+B)");
    italic_btn_ = makeButton("𝐈", "Italic (Ctrl+I)");
    underline_btn_ = makeButton("U̲", "Underline (Ctrl+U)");
    strike_btn_ = makeButton("S̶", "Strikethrough");

    layout->addWidget(bold_btn_);
    layout->addWidget(italic_btn_);
    layout->addWidget(underline_btn_);
    layout->addWidget(strike_btn_);

    layout->addWidget(makeGroupSeparator());

    // Heading group (with subscript numbers for clarity)
    h1_btn_ = makeButton("H₁", "Heading 1");
    h2_btn_ = makeButton("H₂", "Heading 2");
    h3_btn_ = makeButton("H₃", "Heading 3");

    layout->addWidget(h1_btn_);
    layout->addWidget(h2_btn_);
    layout->addWidget(h3_btn_);

    layout->addWidget(makeGroupSeparator());

    // List group (better symbols)
    bullet_btn_ = makeButton("•", "Bullet List");
    numbered_btn_ = makeButton("1.", "Numbered List");

    layout->addWidget(bullet_btn_);
    layout->addWidget(numbered_btn_);

    layout->addWidget(makeGroupSeparator());

    // Block group (improved symbols)
    quote_btn_ = makeButton("❝", "Blockquote");
    code_btn_ = makeButton("⟨⟩", "Code Block");
    hr_btn_ = makeButton("─", "Horizontal Rule");

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
    btn->setFixedSize(kButtonHeightStandard, kButtonHeightStandard);  // 32x32, symmetrical
    return btn;
}

QWidget* FormattingToolbar::makeSeparator() {
    auto* sep = new QWidget(this);
    sep->setObjectName("formatSeparator");
    sep->setFixedSize(1, 20);
    return sep;
}

QWidget* FormattingToolbar::makeGroupSeparator() {
    auto* sep = new QWidget(this);
    sep->setObjectName("formatGroupSeparator");
    sep->setFixedSize(2, 20);  // 2px wide (thicker than normal separator)
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

    // Calculate new state
    ButtonState new_state;
    new_state.bold = (charFmt.fontWeight() == QFont::Bold);
    new_state.italic = charFmt.fontItalic();
    new_state.underline = charFmt.fontUnderline();
    new_state.strike = charFmt.fontStrikeOut();
    new_state.heading_level = blockFmt.headingLevel();

    QTextList* list = cursor.currentList();
    new_state.bullet_list = list && list->format().style() == QTextListFormat::ListDisc;
    new_state.numbered_list = list && list->format().style() == QTextListFormat::ListDecimal;
    new_state.blockquote = blockFmt.property(QTextFormat::BlockQuoteLevel).toInt() > 0;
    new_state.code_block = blockFmt.property(QTextFormat::BlockCodeFence).toBool();

    // Only update buttons that changed state (prevents flicker)
    if (new_state.bold != last_state_.bold)
        setButtonActive(bold_btn_, new_state.bold);
    if (new_state.italic != last_state_.italic)
        setButtonActive(italic_btn_, new_state.italic);
    if (new_state.underline != last_state_.underline)
        setButtonActive(underline_btn_, new_state.underline);
    if (new_state.strike != last_state_.strike)
        setButtonActive(strike_btn_, new_state.strike);

    if (new_state.heading_level != last_state_.heading_level) {
        setButtonActive(h1_btn_, new_state.heading_level == 1);
        setButtonActive(h2_btn_, new_state.heading_level == 2);
        setButtonActive(h3_btn_, new_state.heading_level == 3);
    }

    if (new_state.bullet_list != last_state_.bullet_list)
        setButtonActive(bullet_btn_, new_state.bullet_list);
    if (new_state.numbered_list != last_state_.numbered_list)
        setButtonActive(numbered_btn_, new_state.numbered_list);
    if (new_state.blockquote != last_state_.blockquote)
        setButtonActive(quote_btn_, new_state.blockquote);
    if (new_state.code_block != last_state_.code_block)
        setButtonActive(code_btn_, new_state.code_block);

    last_state_ = new_state;
}

}  // namespace ui
}  // namespace bastionx
