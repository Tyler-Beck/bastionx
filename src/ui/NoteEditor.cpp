#include "bastionx/ui/NoteEditor.h"
#include "bastionx/ui/FormattingToolbar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

namespace bastionx {
namespace ui {

NoteEditor::NoteEditor(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void NoteEditor::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 4);
    layout->setSpacing(0);

    // Title input
    title_input_ = new QLineEdit(this);
    title_input_->setObjectName("titleInput");
    title_input_->setPlaceholderText("Title");
    layout->addWidget(title_input_);

    // Rich text editor
    body_input_ = new QTextEdit(this);
    body_input_->setPlaceholderText("Start writing...");
    body_input_->setAcceptRichText(false);  // Only accept typed text, not pasted HTML

    // Formatting toolbar (needs body_input_ reference)
    formatting_toolbar_ = new FormattingToolbar(body_input_, this);
    layout->addWidget(formatting_toolbar_);
    layout->addWidget(body_input_, 1);

    // Delete button (compact, bottom)
    auto* bottom = new QHBoxLayout();
    bottom->setContentsMargins(0, 4, 0, 0);

    delete_button_ = new QPushButton("DELETE", this);
    delete_button_->setObjectName("deleteButton");
    bottom->addWidget(delete_button_);
    bottom->addStretch();
    layout->addLayout(bottom);

    // Auto-save timer
    autosave_timer_ = new QTimer(this);
    autosave_timer_->setSingleShot(true);
    connect(autosave_timer_, &QTimer::timeout, this, &NoteEditor::onAutoSave);

    // Content change tracking
    connect(title_input_, &QLineEdit::textChanged, this, &NoteEditor::onContentChanged);
    connect(body_input_, &QTextEdit::textChanged, this, &NoteEditor::onContentChanged);
    connect(delete_button_, &QPushButton::clicked, this, &NoteEditor::onDeleteClicked);

    setEditorEnabled(false);
}

void NoteEditor::loadNote(const storage::Note& note) {
    // Block signals while loading to avoid triggering auto-save
    title_input_->blockSignals(true);
    body_input_->blockSignals(true);

    current_note_id_ = note.id;
    title_input_->setText(QString::fromStdString(note.title));
    body_input_->setMarkdown(QString::fromStdString(note.body));

    title_input_->blockSignals(false);
    body_input_->blockSignals(false);

    setModified(false);
    setEditorEnabled(true);
}

bool NoteEditor::saveCurrentNote() {
    if (current_note_id_ == 0 || !repo_ || !subkey_ || !modified_) {
        return false;
    }

    storage::Note note;
    note.id = current_note_id_;
    note.title = title_input_->text().toStdString();
    note.body = body_input_->toMarkdown().toStdString();

    bool ok = repo_->update_note(note, *subkey_);
    if (ok) {
        setModified(false);
    }
    return ok;
}

void NoteEditor::setBackend(storage::NotesRepository* repo, const crypto::SecureKey* subkey) {
    repo_ = repo;
    subkey_ = subkey;
}

void NoteEditor::clearEditor() {
    autosave_timer_->stop();
    title_input_->blockSignals(true);
    body_input_->blockSignals(true);

    title_input_->clear();
    body_input_->clear();
    body_input_->document()->clearUndoRedoStacks();
    current_note_id_ = 0;
    modified_ = false;

    title_input_->blockSignals(false);
    body_input_->blockSignals(false);

    setEditorEnabled(false);
}

bool NoteEditor::hasUnsavedChanges() const {
    return modified_;
}

int64_t NoteEditor::currentNoteId() const {
    return current_note_id_;
}

QString NoteEditor::currentTitle() const {
    return title_input_->text();
}

QString NoteEditor::currentBody() const {
    return body_input_->toMarkdown();
}

void NoteEditor::setTitle(const QString& title) {
    title_input_->blockSignals(true);
    title_input_->setText(title);
    title_input_->blockSignals(false);
}

void NoteEditor::setDocument(QTextDocument* doc) {
    body_input_->blockSignals(true);
    body_input_->setDocument(doc);
    body_input_->blockSignals(false);
}

QTextDocument* NoteEditor::document() const {
    return body_input_->document();
}

void NoteEditor::switchToNote(int64_t note_id, const QString& title) {
    // Switch metadata without touching the document body (preserves undo history)
    autosave_timer_->stop();
    current_note_id_ = note_id;

    title_input_->blockSignals(true);
    title_input_->setText(title);
    title_input_->blockSignals(false);

    setModified(false);
    setEditorEnabled(true);
}

void NoteEditor::onContentChanged() {
    if (current_note_id_ == 0) return;
    setModified(true);
    autosave_timer_->start(kAutoSaveDelayMs);
    emit contentChanged();
}

void NoteEditor::onAutoSave() {
    if (saveCurrentNote()) {
        emit noteSaved();
    }
}

void NoteEditor::onDeleteClicked() {
    if (current_note_id_ == 0 || !repo_) return;

    auto result = QMessageBox::warning(
        this,
        "Delete Note",
        "Delete this note? This cannot be undone.",
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel
    );

    if (result == QMessageBox::Yes) {
        int64_t id = current_note_id_;
        repo_->delete_note(id);
        clearEditor();
        emit noteDeleted(id);
    }
}

void NoteEditor::setModified(bool modified) {
    modified_ = modified;
}

void NoteEditor::setEditorEnabled(bool enabled) {
    title_input_->setEnabled(enabled);
    body_input_->setEnabled(enabled);
    delete_button_->setEnabled(enabled);
    formatting_toolbar_->setEnabled(enabled);
}

}  // namespace ui
}  // namespace bastionx
