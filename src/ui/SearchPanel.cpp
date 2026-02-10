#include "bastionx/ui/SearchPanel.h"
#include <QVBoxLayout>
#include <QDateTime>

namespace bastionx {
namespace ui {

SearchPanel::SearchPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("searchPanel");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    search_input_ = new QLineEdit(this);
    search_input_->setObjectName("searchInput");
    search_input_->setPlaceholderText("Search notes...");
    search_input_->setClearButtonEnabled(true);
    layout->addWidget(search_input_);

    results_count_ = new QLabel(this);
    results_count_->setObjectName("searchResultsCount");
    results_count_->setVisible(false);
    layout->addWidget(results_count_);

    result_list_ = new QListWidget(this);
    result_list_->setObjectName("searchResultList");
    layout->addWidget(result_list_);

    debounce_timer_ = new QTimer(this);
    debounce_timer_->setSingleShot(true);

    connect(search_input_, &QLineEdit::textChanged,
            this, &SearchPanel::onSearchTextChanged);
    connect(debounce_timer_, &QTimer::timeout,
            this, &SearchPanel::onDebounceTimeout);
    connect(result_list_, &QListWidget::itemClicked,
            this, &SearchPanel::onResultClicked);
}

void SearchPanel::setResults(const std::vector<storage::NoteSummary>& results) {
    result_list_->clear();

    for (const auto& s : results) {
        QString title = QString::fromStdString(s.title);
        if (title.trimmed().isEmpty()) title = "(Untitled)";

        QString preview = QString::fromStdString(s.preview).trimmed();
        if (preview.isEmpty()) preview = "(empty)";
        if (preview.length() > 60) preview = preview.left(58) + "..";

        // Relative time
        QString timeStr;
        if (s.updated_at > 0) {
            QDateTime then = QDateTime::fromSecsSinceEpoch(s.updated_at);
            QDateTime now = QDateTime::currentDateTime();
            int secs = static_cast<int>(then.secsTo(now));
            if (secs < 60) timeStr = "just now";
            else if (secs < 3600) timeStr = QString("%1m").arg(secs / 60);
            else if (secs < 86400) timeStr = QString("%1h").arg(secs / 3600);
            else if (secs < 604800) timeStr = QString("%1d").arg(secs / 86400);
            else timeStr = then.toString("MMM d");
        }

        QString displayText = title + "\n" + preview + "  " + timeStr;

        auto* item = new QListWidgetItem(result_list_);
        item->setText(displayText);
        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<qlonglong>(s.id)));
        item->setToolTip(title);
        item->setSizeHint(QSize(0, 52));
    }

    if (results.empty() && !search_input_->text().isEmpty()) {
        results_count_->setText("No results");
    } else if (!results.empty()) {
        results_count_->setText(QString("%1 result%2")
            .arg(results.size())
            .arg(results.size() == 1 ? "" : "s"));
    }
    results_count_->setVisible(!search_input_->text().isEmpty());
}

void SearchPanel::clear() {
    search_input_->clear();
    result_list_->clear();
    results_count_->setVisible(false);
    debounce_timer_->stop();
}

void SearchPanel::onSearchTextChanged() {
    debounce_timer_->start(kDebounceMs);
}

void SearchPanel::onDebounceTimeout() {
    QString query = search_input_->text().trimmed();
    if (query.length() < kMinQueryLen) {
        result_list_->clear();
        results_count_->setVisible(false);
        return;
    }
    emit searchRequested(query);
}

void SearchPanel::onResultClicked(QListWidgetItem* item) {
    int64_t id = item->data(Qt::UserRole).toLongLong();
    emit noteSelected(id);
}

}  // namespace ui
}  // namespace bastionx
