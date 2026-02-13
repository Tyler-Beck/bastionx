#include "bastionx/ui/Sidebar.h"
#include "bastionx/ui/NotesList.h"
#include "bastionx/ui/SearchPanel.h"
#include "bastionx/ui/UIConstants.h"
#include <QVBoxLayout>

namespace bastionx {
namespace ui {

using namespace constants;

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("sidebar");
    setMinimumWidth(kSidebarMinWidth);
    setMaximumWidth(kSidebarMaxWidth);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    stack_ = new QStackedWidget(this);

    // Page 0: Notes list
    notes_list_ = new NotesList(this);
    stack_->addWidget(notes_list_);

    // Page 1: Search panel
    search_panel_ = new SearchPanel(this);
    stack_->addWidget(search_panel_);

    layout->addWidget(stack_);

    // Forward signals from NotesList
    connect(notes_list_, &NotesList::noteSelected,
            this, &Sidebar::noteSelected);
    connect(notes_list_, &NotesList::newNoteRequested,
            this, &Sidebar::newNoteRequested);

    // Forward signals from SearchPanel
    connect(search_panel_, &SearchPanel::noteSelected,
            this, &Sidebar::noteSelected);
    connect(search_panel_, &SearchPanel::searchRequested,
            this, &Sidebar::searchRequested);
}

void Sidebar::setActivity(ActivityBar::Activity activity) {
    switch (activity) {
        case ActivityBar::Notes:
            stack_->setCurrentIndex(0);
            break;
        case ActivityBar::Search:
            stack_->setCurrentIndex(1);
            break;
        case ActivityBar::Settings:
            emit settingsRequested();
            break;
    }
}

}  // namespace ui
}  // namespace bastionx
