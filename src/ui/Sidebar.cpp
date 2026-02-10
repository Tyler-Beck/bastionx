#include "bastionx/ui/Sidebar.h"
#include "bastionx/ui/NotesList.h"
#include <QVBoxLayout>
#include <QLabel>

namespace bastionx {
namespace ui {

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("sidebar");
    setMinimumWidth(kMinWidth);
    setMaximumWidth(kMaxWidth);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    stack_ = new QStackedWidget(this);

    // Page 0: Notes list
    notes_list_ = new NotesList(this);
    stack_->addWidget(notes_list_);

    // Page 1: Search placeholder (replaced in 6C)
    search_placeholder_ = new QWidget(this);
    auto* search_layout = new QVBoxLayout(search_placeholder_);
    auto* search_label = new QLabel("Search (coming soon)", search_placeholder_);
    search_label->setAlignment(Qt::AlignCenter);
    search_label->setStyleSheet("color: #606060; font-size: 12px;");
    search_layout->addWidget(search_label);
    stack_->addWidget(search_placeholder_);

    layout->addWidget(stack_);

    // Forward signals from NotesList
    connect(notes_list_, &NotesList::noteSelected,
            this, &Sidebar::noteSelected);
    connect(notes_list_, &NotesList::newNoteRequested,
            this, &Sidebar::newNoteRequested);
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
