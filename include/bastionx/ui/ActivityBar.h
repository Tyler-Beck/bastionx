#ifndef BASTIONX_UI_ACTIVITYBAR_H
#define BASTIONX_UI_ACTIVITYBAR_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace bastionx {
namespace ui {

class ActivityBar : public QWidget {
    Q_OBJECT

public:
    enum Activity { Notes, Search, Settings };

    explicit ActivityBar(QWidget* parent = nullptr);
    void setActivity(Activity activity);
    Activity currentActivity() const { return current_; }

signals:
    void activityChanged(Activity activity);

private:
    void setupUi();
    void updateButtonStates();

    QPushButton* notes_btn_ = nullptr;
    QPushButton* search_btn_ = nullptr;
    QPushButton* settings_btn_ = nullptr;
    Activity current_ = Notes;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_ACTIVITYBAR_H
