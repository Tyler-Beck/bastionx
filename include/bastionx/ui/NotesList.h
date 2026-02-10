#ifndef BASTIONX_UI_NOTESLIST_H
#define BASTIONX_UI_NOTESLIST_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include "bastionx/storage/NotesRepository.h"

namespace bastionx {
namespace ui {

class NotesList : public QWidget {
    Q_OBJECT

public:
    explicit NotesList(QWidget* parent = nullptr);

    void setSummaries(const std::vector<storage::NoteSummary>& summaries);
    void clear();
    void selectNote(int64_t note_id);

signals:
    void noteSelected(int64_t note_id);
    void newNoteRequested();

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onFilterChanged(const QString& text);

private:
    void setupUi();
    static QString relativeTime(int64_t timestamp);

    QLineEdit*   filter_input_ = nullptr;
    QPushButton* new_button_ = nullptr;
    QListWidget* list_widget_ = nullptr;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_NOTESLIST_H
