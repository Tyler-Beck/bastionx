#ifndef BASTIONX_UI_STATUSBAR_H
#define BASTIONX_UI_STATUSBAR_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

namespace bastionx {
namespace ui {

class StatusBar : public QWidget {
    Q_OBJECT

public:
    explicit StatusBar(QWidget* parent = nullptr);

    void setSaveState(const QString& state);
    void setWordCount(int words, int chars);
    void setEncryptionIndicator(bool encrypted);
    void clear();

private:
    QLabel* save_label_ = nullptr;
    QLabel* encryption_label_ = nullptr;
    QLabel* word_count_label_ = nullptr;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_STATUSBAR_H
