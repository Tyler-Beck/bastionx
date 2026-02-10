#ifndef BASTIONX_UI_SIDEBAR_H
#define BASTIONX_UI_SIDEBAR_H

#include <QWidget>
#include <QStackedWidget>
#include "bastionx/ui/ActivityBar.h"

namespace bastionx {
namespace ui {

class NotesList;

class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);

    void setActivity(ActivityBar::Activity activity);
    NotesList* notesList() const { return notes_list_; }

    // Search panel slot for future 6C integration
    QWidget* searchPlaceholder() const { return search_placeholder_; }

signals:
    void noteSelected(int64_t note_id);
    void newNoteRequested();
    void settingsRequested();

private:
    QStackedWidget* stack_ = nullptr;
    NotesList* notes_list_ = nullptr;
    QWidget* search_placeholder_ = nullptr;

    static constexpr int kMinWidth = 220;
    static constexpr int kMaxWidth = 400;
    static constexpr int kDefaultWidth = 280;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_SIDEBAR_H
