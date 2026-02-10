#include "bastionx/ui/NotesPanel.h"
#include "bastionx/ui/NotesList.h"
#include "bastionx/ui/NoteEditor.h"
#include "bastionx/ui/Sidebar.h"
#include "bastionx/ui/TabBar.h"
#include "bastionx/ui/StatusBar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRegularExpression>

namespace bastionx {
namespace ui {

NotesPanel::NotesPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* outer = new QHBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // Activity bar (far left, 48px)
    activity_bar_ = new ActivityBar(this);
    outer->addWidget(activity_bar_);

    // Splitter: sidebar | editor area
    splitter_ = new QSplitter(Qt::Horizontal, this);

    // Sidebar
    sidebar_ = new Sidebar(this);
    splitter_->addWidget(sidebar_);

    // Editor area (right side)
    editor_area_ = new QWidget(this);
    auto* editor_layout = new QVBoxLayout(editor_area_);
    editor_layout->setContentsMargins(0, 0, 0, 0);
    editor_layout->setSpacing(0);

    tab_bar_ = new TabBar(this);
    editor_layout->addWidget(tab_bar_);

    note_editor_ = new NoteEditor(this);
    editor_layout->addWidget(note_editor_, 1);

    status_bar_ = new StatusBar(this);
    editor_layout->addWidget(status_bar_);

    splitter_->addWidget(editor_area_);

    // Splitter sizing: sidebar fixed-ish, editor stretches
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);
    splitter_->setSizes({280, 650});

    outer->addWidget(splitter_, 1);

    // === Signal connections ===

    // Activity bar -> sidebar
    connect(activity_bar_, &ActivityBar::activityChanged,
            this, &NotesPanel::onActivityChanged);

    // Sidebar -> open note in tab
    connect(sidebar_, &Sidebar::noteSelected,
            this, &NotesPanel::onNoteSelected);
    connect(sidebar_, &Sidebar::newNoteRequested,
            this, &NotesPanel::onNewNoteRequested);
    connect(sidebar_, &Sidebar::settingsRequested,
            this, &NotesPanel::settingsRequested);

    // Tab bar -> switch/close
    connect(tab_bar_, &TabBar::tabSelected,
            this, &NotesPanel::onTabSelected);
    connect(tab_bar_, &TabBar::tabCloseRequested,
            this, &NotesPanel::onTabCloseRequested);

    // Editor -> save/delete
    connect(note_editor_, &NoteEditor::noteSaved,
            this, &NotesPanel::onNoteSaved);
    connect(note_editor_, &NoteEditor::noteDeleted,
            this, &NotesPanel::onNoteDeleted);

    // Track content changes for tab modified indicator
    connect(note_editor_, &NoteEditor::contentChanged,
            this, &NotesPanel::onEditorContentChanged);
}

void NotesPanel::loadNotes(storage::NotesRepository* repo, const crypto::SecureKey* subkey) {
    repo_ = repo;
    subkey_ = subkey;
    note_editor_->setBackend(repo, subkey);
    status_bar_->setEncryptionIndicator(true);
    refreshList();
}

void NotesPanel::prepareForLock() {
    // Save all modified open notes
    for (auto& [id, open_note] : open_notes_) {
        if (open_note.modified && repo_ && subkey_) {
            if (id == active_note_id_) {
                cacheCurrentEditorState();
            }
            repo_->update_note(open_note.note, *subkey_);
        }
    }

    note_editor_->clearEditor();
    tab_bar_->closeAllTabs();
    open_notes_.clear();
    active_note_id_ = 0;
    sidebar_->notesList()->clear();
    status_bar_->clear();
    note_editor_->setBackend(nullptr, nullptr);
    repo_ = nullptr;
    subkey_ = nullptr;
}

void NotesPanel::onActivityChanged(ActivityBar::Activity activity) {
    sidebar_->setActivity(activity);
}

void NotesPanel::onNoteSelected(int64_t note_id) {
    openNoteInTab(note_id);
}

void NotesPanel::onNewNoteRequested() {
    if (!repo_ || !subkey_) return;

    // Save current note before creating new one
    if (active_note_id_ > 0) {
        cacheCurrentEditorState();
        auto it = open_notes_.find(active_note_id_);
        if (it != open_notes_.end() && it->second.modified) {
            note_editor_->saveCurrentNote();
        }
    }

    storage::Note blank;
    blank.title = "";
    blank.body = "";
    int64_t new_id = repo_->create_note(blank, *subkey_);

    refreshList();
    openNoteInTab(new_id);
    sidebar_->notesList()->selectNote(new_id);
}

void NotesPanel::onTabSelected(int64_t note_id) {
    if (note_id == active_note_id_) return;
    switchToTab(note_id);
}

void NotesPanel::onTabCloseRequested(int64_t note_id) {
    auto it = open_notes_.find(note_id);
    if (it != open_notes_.end()) {
        if (it->second.modified && repo_ && subkey_) {
            if (note_id == active_note_id_) {
                cacheCurrentEditorState();
            }
            repo_->update_note(it->second.note, *subkey_);
        }
        open_notes_.erase(it);
    }

    bool was_active = (note_id == active_note_id_);
    tab_bar_->removeTab(note_id);

    if (tab_bar_->tabCount() == 0) {
        active_note_id_ = 0;
        note_editor_->clearEditor();
        status_bar_->setSaveState("");
        status_bar_->setWordCount(0, 0);
    } else if (was_active) {
        // TabBar::removeTab already switched to adjacent tab
        active_note_id_ = tab_bar_->activeNoteId();
        if (active_note_id_ > 0) {
            auto jt = open_notes_.find(active_note_id_);
            if (jt != open_notes_.end()) {
                note_editor_->loadNote(jt->second.note);
                status_bar_->setSaveState(jt->second.modified ? "Modified" : "Saved");
                updateStatusBar();
            }
        }
    }

    refreshList();
}

void NotesPanel::onNoteSaved() {
    if (active_note_id_ > 0) {
        auto it = open_notes_.find(active_note_id_);
        if (it != open_notes_.end()) {
            it->second.modified = false;
            it->second.note.title = note_editor_->currentTitle().toStdString();
            it->second.note.body = note_editor_->currentBody().toStdString();
        }

        tab_bar_->setTabModified(active_note_id_, false);
        tab_bar_->setTabTitle(active_note_id_, note_editor_->currentTitle());
        status_bar_->setSaveState("Saved");
    }
    refreshList();
}

void NotesPanel::onNoteDeleted(int64_t note_id) {
    open_notes_.erase(note_id);
    tab_bar_->removeTab(note_id);

    if (tab_bar_->tabCount() == 0) {
        active_note_id_ = 0;
        status_bar_->setSaveState("");
        status_bar_->setWordCount(0, 0);
    } else {
        active_note_id_ = tab_bar_->activeNoteId();
        if (active_note_id_ > 0) {
            auto jt = open_notes_.find(active_note_id_);
            if (jt != open_notes_.end()) {
                note_editor_->loadNote(jt->second.note);
                status_bar_->setSaveState(jt->second.modified ? "Modified" : "Saved");
                updateStatusBar();
            }
        }
    }
    refreshList();
}

void NotesPanel::onEditorContentChanged() {
    if (active_note_id_ > 0) {
        auto it = open_notes_.find(active_note_id_);
        if (it != open_notes_.end()) {
            it->second.modified = true;
        }
        tab_bar_->setTabModified(active_note_id_, true);
        status_bar_->setSaveState("Modified");
        updateStatusBar();
    }
}

void NotesPanel::refreshList() {
    if (!repo_ || !subkey_) return;
    auto summaries = repo_->list_notes(*subkey_);
    sidebar_->notesList()->setSummaries(summaries);
}

void NotesPanel::openNoteInTab(int64_t note_id) {
    if (!repo_ || !subkey_) return;

    // If already open, just switch to it
    if (tab_bar_->hasTab(note_id)) {
        if (note_id != active_note_id_) {
            switchToTab(note_id);
        }
        tab_bar_->setActiveTab(note_id);
        return;
    }

    // Cache current editor before switching
    if (active_note_id_ > 0) {
        cacheCurrentEditorState();
    }

    // Load note from DB
    auto note = repo_->read_note(note_id, *subkey_);
    if (!note.has_value()) return;

    open_notes_[note_id] = OpenNote{*note, false};

    QString title = QString::fromStdString(note->title);
    if (title.trimmed().isEmpty()) title = "(Untitled)";
    tab_bar_->addTab(note_id, title);

    active_note_id_ = note_id;
    note_editor_->loadNote(*note);
    status_bar_->setSaveState("Saved");
    updateStatusBar();
}

void NotesPanel::cacheCurrentEditorState() {
    if (active_note_id_ == 0) return;

    auto it = open_notes_.find(active_note_id_);
    if (it != open_notes_.end()) {
        it->second.note.title = note_editor_->currentTitle().toStdString();
        it->second.note.body = note_editor_->currentBody().toStdString();
    }
}

void NotesPanel::switchToTab(int64_t note_id) {
    if (note_id == active_note_id_) return;

    cacheCurrentEditorState();

    auto it = open_notes_.find(note_id);
    if (it == open_notes_.end()) return;

    active_note_id_ = note_id;
    tab_bar_->setActiveTab(note_id);
    note_editor_->loadNote(it->second.note);
    status_bar_->setSaveState(it->second.modified ? "Modified" : "Saved");
    updateStatusBar();
}

void NotesPanel::updateStatusBar() {
    QString body = note_editor_->currentBody();
    int chars = body.length();
    int words = 0;
    if (!body.trimmed().isEmpty()) {
        words = static_cast<int>(body.split(QRegularExpression("\\s+"),
                                             Qt::SkipEmptyParts).count());
    }
    status_bar_->setWordCount(words, chars);
}

}  // namespace ui
}  // namespace bastionx
