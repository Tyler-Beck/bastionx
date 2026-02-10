#ifndef BASTIONX_UI_SEARCHPANEL_H
#define BASTIONX_UI_SEARCHPANEL_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include "bastionx/storage/NotesRepository.h"

namespace bastionx {
namespace ui {

class SearchPanel : public QWidget {
    Q_OBJECT

public:
    explicit SearchPanel(QWidget* parent = nullptr);

    void setResults(const std::vector<storage::NoteSummary>& results);
    void clear();

signals:
    void searchRequested(const QString& query);
    void noteSelected(int64_t note_id);

private slots:
    void onSearchTextChanged();
    void onDebounceTimeout();
    void onResultClicked(QListWidgetItem* item);

private:
    QLineEdit* search_input_ = nullptr;
    QLabel* results_count_ = nullptr;
    QListWidget* result_list_ = nullptr;
    QTimer* debounce_timer_ = nullptr;

    static constexpr int kDebounceMs = 300;
    static constexpr int kMinQueryLen = 2;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_SEARCHPANEL_H
