#include "bastionx/ui/NotesList.h"
#include <QVBoxLayout>

namespace bastionx {
namespace ui {

NotesList::NotesList(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void NotesList::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    new_button_ = new QPushButton("+ NEW NOTE", this);
    new_button_->setObjectName("newNoteButton");
    layout->addWidget(new_button_);

    list_widget_ = new QListWidget(this);
    layout->addWidget(list_widget_);

    connect(new_button_, &QPushButton::clicked,
            this, &NotesList::newNoteRequested);
    connect(list_widget_, &QListWidget::itemClicked,
            this, &NotesList::onItemClicked);
}

void NotesList::setSummaries(const std::vector<storage::NoteSummary>& summaries) {
    list_widget_->clear();
    for (const auto& s : summaries) {
        QString title = QString::fromStdString(s.title);
        if (title.trimmed().isEmpty()) {
            title = "(Untitled)";
        }
        auto* item = new QListWidgetItem(title, list_widget_);
        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<qlonglong>(s.id)));
    }
}

void NotesList::clear() {
    list_widget_->clear();
}

void NotesList::selectNote(int64_t note_id) {
    for (int i = 0; i < list_widget_->count(); ++i) {
        auto* item = list_widget_->item(i);
        if (item->data(Qt::UserRole).toLongLong() == note_id) {
            list_widget_->setCurrentItem(item);
            return;
        }
    }
}

void NotesList::onItemClicked(QListWidgetItem* item) {
    int64_t id = item->data(Qt::UserRole).toLongLong();
    emit noteSelected(id);
}

}  // namespace ui
}  // namespace bastionx
