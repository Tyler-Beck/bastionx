#ifndef BASTIONX_UI_TABBAR_H
#define BASTIONX_UI_TABBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QPropertyAnimation>
#include <vector>
#include <cstdint>

namespace bastionx {
namespace ui {

class TabBar : public QWidget {
    Q_OBJECT

public:
    explicit TabBar(QWidget* parent = nullptr);

    int addTab(int64_t note_id, const QString& title);
    void removeTab(int64_t note_id);
    void setActiveTab(int64_t note_id);
    void setTabTitle(int64_t note_id, const QString& title);
    void setTabModified(int64_t note_id, bool modified);
    bool hasTab(int64_t note_id) const;
    int64_t activeNoteId() const { return active_note_id_; }
    std::vector<int64_t> openNoteIds() const;
    void closeAllTabs();
    int tabCount() const;

signals:
    void tabSelected(int64_t note_id);
    void tabCloseRequested(int64_t note_id);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    struct TabInfo {
        int64_t note_id;
        QString title;
        bool modified;
        QPushButton* button;
        QPushButton* close_button;
        QWidget* container;
        QFrame* right_separator = nullptr;
    };

    void updateTabStyles();
    QString tabLabel(const TabInfo& info) const;
    void updateBladeGeometry(bool animated = true);
    QRect getTabGeometry(int64_t note_id) const;

    QScrollArea* scroll_area_ = nullptr;
    QWidget* scroll_content_ = nullptr;
    QHBoxLayout* tab_layout_ = nullptr;
    std::vector<TabInfo> tabs_;
    int64_t active_note_id_ = 0;

    QFrame* blade_indicator_ = nullptr;
    QPropertyAnimation* blade_animation_ = nullptr;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_TABBAR_H
