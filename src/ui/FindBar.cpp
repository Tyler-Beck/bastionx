#include "bastionx/ui/FindBar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextCursor>
#include <QTextBlock>
#include <QTimer>

namespace bastionx {
namespace ui {

FindBar::FindBar(QTextEdit* editor, QWidget* parent)
    : QWidget(parent), editor_(editor)
{
    setObjectName("findBar");
    setFixedHeight(64);
    hide();  // Start hidden

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 4, 8, 4);
    outer->setSpacing(2);

    // Find row
    auto* find_row = new QHBoxLayout();
    find_row->setSpacing(4);

    find_input_ = new QLineEdit(this);
    find_input_->setObjectName("findInput");
    find_input_->setPlaceholderText("Find...");
    find_row->addWidget(find_input_, 1);

    match_label_ = new QLabel("0/0", this);
    match_label_->setObjectName("matchLabel");
    match_label_->setFixedWidth(48);
    match_label_->setAlignment(Qt::AlignCenter);
    find_row->addWidget(match_label_);

    prev_btn_ = new QPushButton("\xe2\x96\xb2", this);  // Up triangle
    prev_btn_->setObjectName("findButton");
    prev_btn_->setFixedSize(24, 24);
    find_row->addWidget(prev_btn_);

    next_btn_ = new QPushButton("\xe2\x96\xbc", this);  // Down triangle
    next_btn_->setObjectName("findButton");
    next_btn_->setFixedSize(24, 24);
    find_row->addWidget(next_btn_);

    close_btn_ = new QPushButton("x", this);
    close_btn_->setObjectName("findButton");
    close_btn_->setFixedSize(24, 24);
    find_row->addWidget(close_btn_);

    outer->addLayout(find_row);

    // Replace row (initially hidden)
    replace_row_ = new QWidget(this);
    auto* replace_layout = new QHBoxLayout(replace_row_);
    replace_layout->setContentsMargins(0, 0, 0, 0);
    replace_layout->setSpacing(4);

    replace_input_ = new QLineEdit(replace_row_);
    replace_input_->setObjectName("replaceInput");
    replace_input_->setPlaceholderText("Replace...");
    replace_layout->addWidget(replace_input_, 1);

    replace_btn_ = new QPushButton("Replace", replace_row_);
    replace_btn_->setObjectName("findButton");
    replace_layout->addWidget(replace_btn_);

    replace_all_btn_ = new QPushButton("All", replace_row_);
    replace_all_btn_->setObjectName("findButton");
    replace_layout->addWidget(replace_all_btn_);

    replace_row_->hide();
    outer->addWidget(replace_row_);

    // Connections
    connect(find_input_, &QLineEdit::textChanged, this, &FindBar::onFindTextChanged);
    connect(find_input_, &QLineEdit::returnPressed, this, &FindBar::onFindNext);
    connect(next_btn_, &QPushButton::clicked, this, &FindBar::onFindNext);
    connect(prev_btn_, &QPushButton::clicked, this, &FindBar::onFindPrev);
    connect(replace_btn_, &QPushButton::clicked, this, &FindBar::onReplace);
    connect(replace_all_btn_, &QPushButton::clicked, this, &FindBar::onReplaceAll);
    connect(close_btn_, &QPushButton::clicked, this, &FindBar::hideBar);

    // Create reusable height animations
    height_animation_ = new QPropertyAnimation(this, "maximumHeight", this);
    height_animation_->setDuration(200);
    height_animation_->setEasingCurve(QEasingCurve::OutCubic);

    height_animation_min_ = new QPropertyAnimation(this, "minimumHeight", this);
    height_animation_min_->setDuration(200);
    height_animation_min_->setEasingCurve(QEasingCurve::OutCubic);
}

void FindBar::showFind() {
    replace_row_->hide();

    // Animate to find-only height
    height_animation_->setStartValue(height());
    height_animation_->setEndValue(32);
    height_animation_min_->setStartValue(height());
    height_animation_min_->setEndValue(32);
    height_animation_->start();
    height_animation_min_->start();

    show();
    find_input_->setFocus();
    find_input_->selectAll();
}

void FindBar::showReplace() {
    // Animate to replace height
    height_animation_->setStartValue(height());
    height_animation_->setEndValue(64);
    height_animation_min_->setStartValue(height());
    height_animation_min_->setEndValue(64);
    height_animation_->start();
    height_animation_min_->start();

    // Show replace row after animation starts
    QTimer::singleShot(50, [this]() {
        replace_row_->show();
    });

    show();
    find_input_->setFocus();
    find_input_->selectAll();
}

void FindBar::hideBar() {
    clearHighlights();
    hide();
}

void FindBar::onFindTextChanged(const QString& text) {
    if (text.isEmpty()) {
        clearHighlights();
        match_label_->setText("0/0");
        current_match_ = 0;
        total_matches_ = 0;
        return;
    }

    highlightAllMatches();

    // Move cursor to first match
    QTextCursor cursor = editor_->textCursor();
    cursor.movePosition(QTextCursor::Start);
    editor_->setTextCursor(cursor);

    current_match_ = 0;
    if (total_matches_ > 0) {
        onFindNext();
    }
}

void FindBar::onFindNext() {
    QString text = find_input_->text();
    if (text.isEmpty() || total_matches_ == 0) return;

    QTextCursor cursor = editor_->textCursor();
    QTextCursor found = editor_->document()->find(text, cursor);

    if (found.isNull()) {
        // Wrap around to beginning
        cursor.movePosition(QTextCursor::Start);
        found = editor_->document()->find(text, cursor);
        current_match_ = 1;
    } else {
        current_match_++;
        if (current_match_ > total_matches_) current_match_ = 1;
    }

    if (!found.isNull()) {
        editor_->setTextCursor(found);
    }

    updateMatchLabel();
}

void FindBar::onFindPrev() {
    QString text = find_input_->text();
    if (text.isEmpty() || total_matches_ == 0) return;

    QTextCursor cursor = editor_->textCursor();
    // Move to start of current selection so we don't find the same match
    cursor.setPosition(cursor.selectionStart());
    QTextCursor found = editor_->document()->find(text, cursor, QTextDocument::FindBackward);

    if (found.isNull()) {
        // Wrap around to end
        cursor.movePosition(QTextCursor::End);
        found = editor_->document()->find(text, cursor, QTextDocument::FindBackward);
        current_match_ = total_matches_;
    } else {
        current_match_--;
        if (current_match_ < 1) current_match_ = total_matches_;
    }

    if (!found.isNull()) {
        editor_->setTextCursor(found);
    }

    updateMatchLabel();
}

void FindBar::onReplace() {
    QString find_text = find_input_->text();
    if (find_text.isEmpty()) return;

    QTextCursor cursor = editor_->textCursor();
    if (cursor.hasSelection() &&
        cursor.selectedText().compare(find_text, Qt::CaseInsensitive) == 0) {
        cursor.insertText(replace_input_->text());
        highlightAllMatches();
    }

    onFindNext();
}

void FindBar::onReplaceAll() {
    QString find_text = find_input_->text();
    QString replace_text = replace_input_->text();
    if (find_text.isEmpty()) return;

    QTextCursor cursor(editor_->document());
    cursor.beginEditBlock();

    cursor.movePosition(QTextCursor::Start);
    while (true) {
        QTextCursor found = editor_->document()->find(find_text, cursor);
        if (found.isNull()) break;
        found.insertText(replace_text);
        cursor = found;
    }

    cursor.endEditBlock();
    highlightAllMatches();
    updateMatchLabel();
}

void FindBar::highlightAllMatches() {
    clearHighlights();

    QString text = find_input_->text();
    if (text.isEmpty()) {
        total_matches_ = 0;
        updateMatchLabel();
        return;
    }

    QList<QTextEdit::ExtraSelection> selections;
    QTextCursor cursor(editor_->document());

    QTextCharFormat fmt;
    fmt.setBackground(QColor("#5a5a2a"));  // Yellow-ish highlight for dark theme

    while (true) {
        QTextCursor found = editor_->document()->find(text, cursor);
        if (found.isNull()) break;

        QTextEdit::ExtraSelection sel;
        sel.cursor = found;
        sel.format = fmt;
        selections.append(sel);

        cursor = found;
    }

    editor_->setExtraSelections(selections);
    total_matches_ = selections.size();
    updateMatchLabel();
}

void FindBar::updateMatchLabel() {
    if (total_matches_ == 0) {
        match_label_->setText("0/0");
    } else {
        match_label_->setText(QString("%1/%2").arg(current_match_).arg(total_matches_));
    }
}

void FindBar::clearHighlights() {
    editor_->setExtraSelections({});
    total_matches_ = 0;
    current_match_ = 0;
}

int FindBar::countMatches() const {
    return total_matches_;
}

}  // namespace ui
}  // namespace bastionx
