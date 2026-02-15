#ifndef BASTIONX_UI_SIDEBAR_H
#define BASTIONX_UI_SIDEBAR_H

#include <QWidget>
#include <QStackedWidget>
#include "bastionx/ui/ActivityBar.h"
#include "bastionx/ui/ModeSelectorBar.h"

namespace bastionx {
namespace ui {

class NotesList;
class SearchPanel;

class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);

    void setActivity(ActivityBar::Activity activity);
    NotesList* notesList() const { return notes_list_; }
    SearchPanel* searchPanel() const { return search_panel_; }

signals:
    void noteSelected(int64_t note_id);
    void newNoteRequested();
    void settingsRequested();
    void searchRequested(const QString& query);

private slots:
    void onActivityChanged(ModeSelectorBar::Activity activity);

private:
    ModeSelectorBar* mode_selector_ = nullptr;
    QStackedWidget* stack_ = nullptr;
    NotesList* notes_list_ = nullptr;
    SearchPanel* search_panel_ = nullptr;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_SIDEBAR_H
