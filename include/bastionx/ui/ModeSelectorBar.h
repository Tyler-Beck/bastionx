#ifndef BASTIONX_UI_MODESELECTORBAR_H
#define BASTIONX_UI_MODESELECTORBAR_H

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>

namespace bastionx {
namespace ui {

class ModeSelectorBar : public QWidget {
    Q_OBJECT

public:
    enum Activity { Notes, Search, Settings };

    explicit ModeSelectorBar(QWidget* parent = nullptr);
    void setActivity(Activity activity);
    Activity currentActivity() const { return current_; }

signals:
    void activityChanged(Activity activity);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void updateSegmentStates();
    void animateBladeTo(Activity activity);

    QLabel* notes_label_ = nullptr;
    QLabel* search_label_ = nullptr;
    QLabel* settings_label_ = nullptr;
    QFrame* blade_indicator_ = nullptr;
    Activity current_ = Notes;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_MODESELECTORBAR_H
