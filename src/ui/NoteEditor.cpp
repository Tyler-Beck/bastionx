#include "bastionx/ui/NoteEditor.h"
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
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    title_input_ = new QLineEdit(this);
    title_input_->setObjectName("titleInput");
    title_input_->setPlaceholderText("Title");
    layout->addWidget(title_input_);

    body_input_ = new QPlainTextEdit(this);
    body_input_->setPlaceholderText("Start writing...");
    layout->addWidget(body_input_, 1);

    // Bottom bar
    auto* bottom = new QHBoxLayout();
    bottom->setContentsMargins(0, 4, 0, 0);

    delete_button_ = new QPushButton("DELETE", this);
    delete_button_->setObjectName("deleteButton");
    bottom->addWidget(delete_button_);

    bottom->addStretch();

    status_label_ = new QLabel("", this);
    status_label_->setObjectName("statusLabel");
    bottom->addWidget(status_label_);

    layout->addLayout(bottom);

    // Auto-save timer
    autosave_timer_ = new QTimer(this);
    autosave_timer_->setSingleShot(true);
    connect(autosave_timer_, &QTimer::timeout, this, &NoteEditor::onAutoSave);

    // Content change tracking
    connect(title_input_, &QLineEdit::textChanged, this, &NoteEditor::onContentChanged);
    connect(body_input_, &QPlainTextEdit::textChanged, this, &NoteEditor::onContentChanged);
    connect(delete_button_, &QPushButton::clicked, this, &NoteEditor::onDeleteClicked);

    setEditorEnabled(false);
}

void NoteEditor::loadNote(const storage::Note& note) {
    // Block signals while loading to avoid triggering auto-save
    title_input_->blockSignals(true);
    body_input_->blockSignals(true);

    current_note_id_ = note.id;
    title_input_->setText(QString::fromStdString(note.title));
    body_input_->setPlainText(QString::fromStdString(note.body));

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
    note.body = body_input_->toPlainText().toStdString();

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
    current_note_id_ = 0;
    modified_ = false;
    status_label_->clear();

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
    return body_input_->toPlainText();
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
    status_label_->setText(modified ? "Modified" : "Saved");
}

void NoteEditor::setEditorEnabled(bool enabled) {
    title_input_->setEnabled(enabled);
    body_input_->setEnabled(enabled);
    delete_button_->setEnabled(enabled);
    if (!enabled) {
        status_label_->clear();
    }
}

}  // namespace ui
}  // namespace bastionx
