#ifndef BASTIONX_UI_NOTESPANEL_H
#define BASTIONX_UI_NOTESPANEL_H

#include <QWidget>
#include <QSplitter>
#include <QTextDocument>
#include <map>
#include "bastionx/storage/NotesRepository.h"
#include "bastionx/crypto/SecureMemory.h"

namespace bastionx {
namespace ui {

class NotesList;
class NoteEditor;
class Sidebar;
class TabBar;
class StatusBar;

class NotesPanel : public QWidget {
    Q_OBJECT

public:
    explicit NotesPanel(QWidget* parent = nullptr);

    void loadNotes(storage::NotesRepository* repo, const crypto::SecureKey* subkey);
    void prepareForLock();

signals:
    void settingsRequested();

private slots:
    void onNoteSelected(int64_t note_id);
    void onNewNoteRequested();
    void onTabSelected(int64_t note_id);
    void onTabCloseRequested(int64_t note_id);
    void onNoteSaved();
    void onNoteDeleted(int64_t note_id);
    void onEditorContentChanged();
    void onSearchRequested(const QString& query);

private:
    void refreshList();
    void openNoteInTab(int64_t note_id);
    void cacheCurrentEditorState();
    void switchToTab(int64_t note_id);
    void updateStatusBar();

    // In-memory cache of open notes (per-tab QTextDocument for undo history)
    struct OpenNote {
        storage::Note note;
        bool modified = false;
        QTextDocument* document = nullptr;
    };

    // Layout
    Sidebar* sidebar_ = nullptr;
    QSplitter* splitter_ = nullptr;
    QWidget* editor_area_ = nullptr;
    TabBar* tab_bar_ = nullptr;
    NoteEditor* note_editor_ = nullptr;
    StatusBar* status_bar_ = nullptr;

    // State
    std::map<int64_t, OpenNote> open_notes_;
    int64_t active_note_id_ = 0;

    // Backend
    storage::NotesRepository* repo_ = nullptr;
    const crypto::SecureKey* subkey_ = nullptr;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_NOTESPANEL_H
