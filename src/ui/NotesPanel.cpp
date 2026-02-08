#include "bastionx/ui/NotesPanel.h"
#include "bastionx/ui/NotesList.h"
#include "bastionx/ui/NoteEditor.h"
#include <QHBoxLayout>

namespace bastionx {
namespace ui {

NotesPanel::NotesPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    splitter_ = new QSplitter(Qt::Horizontal, this);

    notes_list_ = new NotesList(this);
    notes_list_->setMinimumWidth(200);
    notes_list_->setMaximumWidth(350);
    splitter_->addWidget(notes_list_);

    note_editor_ = new NoteEditor(this);
    splitter_->addWidget(note_editor_);

    splitter_->setStretchFactor(0, 0);  // list: fixed
    splitter_->setStretchFactor(1, 1);  // editor: stretches
    splitter_->setSizes({250, 650});

    layout->addWidget(splitter_);

    // Connections
    connect(notes_list_, &NotesList::noteSelected,
            this, &NotesPanel::onNoteSelected);
    connect(notes_list_, &NotesList::newNoteRequested,
            this, &NotesPanel::onNewNoteRequested);
    connect(note_editor_, &NoteEditor::noteSaved,
            this, &NotesPanel::onNoteSaved);
    connect(note_editor_, &NoteEditor::noteDeleted,
            this, &NotesPanel::onNoteDeleted);
}

void NotesPanel::loadNotes(storage::NotesRepository* repo, const crypto::SecureKey* subkey) {
    repo_ = repo;
    subkey_ = subkey;
    note_editor_->setBackend(repo, subkey);
    refreshList();
}

void NotesPanel::prepareForLock() {
    note_editor_->saveCurrentNote();
    note_editor_->clearEditor();
    notes_list_->clear();
    note_editor_->setBackend(nullptr, nullptr);
    repo_ = nullptr;
    subkey_ = nullptr;
}

void NotesPanel::onNoteSelected(int64_t note_id) {
    if (!repo_ || !subkey_) return;

    // Save current note before switching
    note_editor_->saveCurrentNote();

    auto note = repo_->read_note(note_id, *subkey_);
    if (note.has_value()) {
        note_editor_->loadNote(*note);
    }
}

void NotesPanel::onNewNoteRequested() {
    if (!repo_ || !subkey_) return;

    // Save current note first
    note_editor_->saveCurrentNote();

    // Create blank note
    storage::Note blank;
    blank.title = "";
    blank.body = "";
    int64_t new_id = repo_->create_note(blank, *subkey_);

    refreshList();

    // Load the new note into editor
    auto note = repo_->read_note(new_id, *subkey_);
    if (note.has_value()) {
        note_editor_->loadNote(*note);
        notes_list_->selectNote(new_id);
    }
}

void NotesPanel::onNoteSaved() {
    refreshList();
    // Re-select the current note in the list
    int64_t current_id = note_editor_->currentNoteId();
    if (current_id > 0) {
        notes_list_->selectNote(current_id);
    }
}

void NotesPanel::onNoteDeleted(int64_t /*note_id*/) {
    refreshList();
}

void NotesPanel::refreshList() {
    if (!repo_ || !subkey_) return;
    auto summaries = repo_->list_notes(*subkey_);
    notes_list_->setSummaries(summaries);
}

}  // namespace ui
}  // namespace bastionx
