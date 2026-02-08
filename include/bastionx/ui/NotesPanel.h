#ifndef BASTIONX_UI_NOTESPANEL_H
#define BASTIONX_UI_NOTESPANEL_H

#include <QWidget>
#include <QSplitter>
#include "bastionx/storage/NotesRepository.h"
#include "bastionx/crypto/SecureMemory.h"

namespace bastionx {
namespace ui {

class NotesList;
class NoteEditor;

class NotesPanel : public QWidget {
    Q_OBJECT

public:
    explicit NotesPanel(QWidget* parent = nullptr);

    void loadNotes(storage::NotesRepository* repo, const crypto::SecureKey* subkey);
    void prepareForLock();

private slots:
    void onNoteSelected(int64_t note_id);
    void onNewNoteRequested();
    void onNoteSaved();
    void onNoteDeleted(int64_t note_id);

private:
    void refreshList();

    QSplitter*  splitter_ = nullptr;
    NotesList*  notes_list_ = nullptr;
    NoteEditor* note_editor_ = nullptr;

    storage::NotesRepository* repo_ = nullptr;
    const crypto::SecureKey*  subkey_ = nullptr;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_NOTESPANEL_H
