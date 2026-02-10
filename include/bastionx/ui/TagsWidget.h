#ifndef BASTIONX_UI_TAGSWIDGET_H
#define BASTIONX_UI_TAGSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <vector>
#include <string>

namespace bastionx {
namespace ui {

class TagsWidget : public QWidget {
    Q_OBJECT

public:
    explicit TagsWidget(QWidget* parent = nullptr);

    void setTags(const std::vector<std::string>& tags);
    std::vector<std::string> tags() const;
    void clear();

signals:
    void tagsChanged();

private slots:
    void onAddTag();
    void onRemoveTag();

private:
    void rebuildChips();

    QHBoxLayout* chip_layout_ = nullptr;
    QLineEdit* add_input_ = nullptr;
    std::vector<std::string> tags_;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_TAGSWIDGET_H
