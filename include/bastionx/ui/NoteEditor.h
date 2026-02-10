#ifndef BASTIONX_UI_NOTEEDITOR_H
#define BASTIONX_UI_NOTEEDITOR_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QTextDocument>
#include "bastionx/storage/NotesRepository.h"
#include "bastionx/crypto/SecureMemory.h"

namespace bastionx {
namespace ui {

class FormattingToolbar;

class NoteEditor : public QWidget {
    Q_OBJECT

public:
    explicit NoteEditor(QWidget* parent = nullptr);

    void loadNote(const storage::Note& note);
    bool saveCurrentNote();
    void setBackend(storage::NotesRepository* repo, const crypto::SecureKey* subkey);
    void clearEditor();
    bool hasUnsavedChanges() const;
    int64_t currentNoteId() const;

    QString currentTitle() const;
    QString currentBody() const;

    void setTitle(const QString& title);
    void setDocument(QTextDocument* doc);
    QTextDocument* document() const;
    void switchToNote(int64_t note_id, const QString& title);

signals:
    void noteSaved();
    void noteDeleted(int64_t note_id);
    void contentChanged();

private slots:
    void onContentChanged();
    void onAutoSave();
    void onDeleteClicked();

private:
    void setupUi();
    void setModified(bool modified);
    void setEditorEnabled(bool enabled);

    QLineEdit*          title_input_ = nullptr;
    FormattingToolbar*  formatting_toolbar_ = nullptr;
    QTextEdit*          body_input_ = nullptr;
    QPushButton*        delete_button_ = nullptr;
    QTimer*             autosave_timer_ = nullptr;

    int64_t current_note_id_ = 0;
    bool modified_ = false;

    storage::NotesRepository* repo_ = nullptr;
    const crypto::SecureKey*  subkey_ = nullptr;

    static constexpr int kAutoSaveDelayMs = 2000;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_NOTEEDITOR_H
