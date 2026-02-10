#include "bastionx/ui/NotesList.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>

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

    filter_input_ = new QLineEdit(this);
    filter_input_->setObjectName("filterInput");
    filter_input_->setPlaceholderText("Filter notes...");
    filter_input_->setClearButtonEnabled(true);
    layout->addWidget(filter_input_);

    new_button_ = new QPushButton("+ NEW NOTE", this);
    new_button_->setObjectName("newNoteButton");
    layout->addWidget(new_button_);

    list_widget_ = new QListWidget(this);
    layout->addWidget(list_widget_);

    connect(new_button_, &QPushButton::clicked,
            this, &NotesList::newNoteRequested);
    connect(list_widget_, &QListWidget::itemClicked,
            this, &NotesList::onItemClicked);
    connect(filter_input_, &QLineEdit::textChanged,
            this, &NotesList::onFilterChanged);
}

void NotesList::setSummaries(const std::vector<storage::NoteSummary>& summaries) {
    list_widget_->clear();

    for (const auto& s : summaries) {
        QString title = QString::fromStdString(s.title);
        if (title.trimmed().isEmpty()) {
            title = "(Untitled)";
        }

        // Build preview text
        QString preview = QString::fromStdString(s.preview).trimmed();
        if (preview.isEmpty()) preview = "(empty)";
        if (preview.length() > 60) {
            preview = preview.left(58) + "..";
        }

        QString timeStr = relativeTime(s.updated_at);

        // Multi-line item: title on first line, preview + time below
        QString displayText = title + "\n" + preview + "  " + timeStr;

        auto* item = new QListWidgetItem(list_widget_);
        item->setText(displayText);
        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<qlonglong>(s.id)));
        item->setToolTip(title);
        item->setSizeHint(QSize(0, 52));
    }

    // Re-apply filter if active
    if (!filter_input_->text().isEmpty()) {
        onFilterChanged(filter_input_->text());
    }
}

void NotesList::clear() {
    list_widget_->clear();
    filter_input_->clear();
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

void NotesList::onFilterChanged(const QString& text) {
    for (int i = 0; i < list_widget_->count(); ++i) {
        auto* item = list_widget_->item(i);
        bool visible = text.isEmpty() ||
                       item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!visible);
    }
}

QString NotesList::relativeTime(int64_t timestamp) {
    if (timestamp == 0) return "";

    QDateTime then = QDateTime::fromSecsSinceEpoch(timestamp);
    QDateTime now = QDateTime::currentDateTime();
    int secs = static_cast<int>(then.secsTo(now));

    if (secs < 0) return "just now";
    if (secs < 60) return "just now";
    if (secs < 3600) return QString("%1m").arg(secs / 60);
    if (secs < 86400) return QString("%1h").arg(secs / 3600);
    if (secs < 604800) return QString("%1d").arg(secs / 86400);
    if (secs < 2592000) return QString("%1w").arg(secs / 604800);
    return then.toString("MMM d");
}

}  // namespace ui
}  // namespace bastionx
