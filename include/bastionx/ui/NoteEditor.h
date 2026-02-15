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
class TagsWidget;
class FindBar;

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
    std::vector<std::string> currentTags() const;

    void setTitle(const QString& title);
    void setTags(const std::vector<std::string>& tags);
    void setDocument(QTextDocument* doc);
    QTextDocument* document() const;
    void switchToNote(int64_t note_id, const QString& title,
                      const std::vector<std::string>& tags);

    void showFindBar();
    void showReplaceBar();

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
    TagsWidget*         tags_widget_ = nullptr;
    FormattingToolbar*  formatting_toolbar_ = nullptr;
    FindBar*            find_bar_ = nullptr;
    QTextEdit*          body_input_ = nullptr;
    QPushButton*        delete_button_ = nullptr;
    QLabel*             status_label_ = nullptr;
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
