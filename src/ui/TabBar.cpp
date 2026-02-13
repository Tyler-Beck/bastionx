#include "bastionx/ui/TabBar.h"
#include "bastionx/ui/UIConstants.h"
#include <QStyle>
#include <algorithm>

namespace bastionx {
namespace ui {

using namespace constants;

TabBar::TabBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("tabBar");
    setFixedHeight(kTabBarHeight);

    auto* outer = new QHBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    scroll_area_ = new QScrollArea(this);
    scroll_area_->setWidgetResizable(true);
    scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area_->setFrameShape(QFrame::NoFrame);
    scroll_area_->setObjectName("tabScrollArea");

    scroll_content_ = new QWidget(scroll_area_);
    tab_layout_ = new QHBoxLayout(scroll_content_);
    tab_layout_->setContentsMargins(kMarginTiny, 0, 0, 0);  // 4px left padding
    tab_layout_->setSpacing(2);  // 2px gap between tabs
    tab_layout_->addStretch();

    scroll_area_->setWidget(scroll_content_);
    outer->addWidget(scroll_area_);
}

int TabBar::addTab(int64_t note_id, const QString& title) {
    // Don't add duplicate tabs
    if (hasTab(note_id)) {
        setActiveTab(note_id);
        return -1;
    }

    TabInfo info;
    info.note_id = note_id;
    info.title = title;
    info.modified = false;

    // Container widget for tab button + close button
    info.container = new QWidget(scroll_content_);
    auto* container_layout = new QHBoxLayout(info.container);
    container_layout->setContentsMargins(0, 0, 0, 0);
    container_layout->setSpacing(0);

    // Tab title button
    info.button = new QPushButton(tabLabel(info), info.container);
    info.button->setCursor(Qt::PointingHandCursor);
    info.button->setFlat(true);
    info.button->setMinimumWidth(kTabMinWidth);
    info.button->setMaximumWidth(kTabMaxWidth);
    container_layout->addWidget(info.button);

    // Close button
    info.close_button = new QPushButton("x", info.container);
    info.close_button->setObjectName("tabCloseButton");
    info.close_button->setCursor(Qt::PointingHandCursor);
    info.close_button->setFlat(true);
    info.close_button->setFixedSize(kCloseButtonSize, kCloseButtonSize);
    container_layout->addWidget(info.close_button);

    // Insert before the stretch spacer
    int insert_pos = tab_layout_->count() - 1;  // before the stretch
    tab_layout_->insertWidget(insert_pos, info.container);

    // Connections
    int64_t id = note_id;
    connect(info.button, &QPushButton::clicked, this, [this, id] {
        setActiveTab(id);
        emit tabSelected(id);
    });
    connect(info.close_button, &QPushButton::clicked, this, [this, id] {
        emit tabCloseRequested(id);
    });

    tabs_.push_back(std::move(info));

    setActiveTab(note_id);
    return static_cast<int>(tabs_.size()) - 1;
}

void TabBar::removeTab(int64_t note_id) {
    auto it = std::find_if(tabs_.begin(), tabs_.end(),
        [note_id](const TabInfo& t) { return t.note_id == note_id; });
    if (it == tabs_.end()) return;

    // Remove the container widget from layout
    tab_layout_->removeWidget(it->container);
    delete it->container;  // Deletes button + close_button too
    tabs_.erase(it);

    // If we removed the active tab, switch to the last tab (or clear)
    if (active_note_id_ == note_id) {
        if (!tabs_.empty()) {
            active_note_id_ = tabs_.back().note_id;
            updateTabStyles();
            emit tabSelected(active_note_id_);
        } else {
            active_note_id_ = 0;
            updateTabStyles();
        }
    }
}

void TabBar::setActiveTab(int64_t note_id) {
    active_note_id_ = note_id;
    updateTabStyles();
}

void TabBar::setTabTitle(int64_t note_id, const QString& title) {
    for (auto& tab : tabs_) {
        if (tab.note_id == note_id) {
            tab.title = title;
            tab.button->setText(tabLabel(tab));
            return;
        }
    }
}

void TabBar::setTabModified(int64_t note_id, bool modified) {
    for (auto& tab : tabs_) {
        if (tab.note_id == note_id) {
            tab.modified = modified;
            tab.button->setText(tabLabel(tab));
            return;
        }
    }
}

bool TabBar::hasTab(int64_t note_id) const {
    return std::any_of(tabs_.begin(), tabs_.end(),
        [note_id](const TabInfo& t) { return t.note_id == note_id; });
}

std::vector<int64_t> TabBar::openNoteIds() const {
    std::vector<int64_t> ids;
    ids.reserve(tabs_.size());
    for (const auto& tab : tabs_) {
        ids.push_back(tab.note_id);
    }
    return ids;
}

void TabBar::closeAllTabs() {
    for (auto& tab : tabs_) {
        tab_layout_->removeWidget(tab.container);
        delete tab.container;
    }
    tabs_.clear();
    active_note_id_ = 0;
}

int TabBar::tabCount() const {
    return static_cast<int>(tabs_.size());
}

void TabBar::updateTabStyles() {
    for (auto& tab : tabs_) {
        bool active = (tab.note_id == active_note_id_);
        tab.button->setObjectName(active ? "tabActive" : "tab");
        tab.button->style()->unpolish(tab.button);
        tab.button->style()->polish(tab.button);
        tab.container->setObjectName(active ? "tabContainerActive" : "tabContainer");
        tab.container->style()->unpolish(tab.container);
        tab.container->style()->polish(tab.container);
    }
}

QString TabBar::tabLabel(const TabInfo& info) const {
    QString label = info.title.isEmpty() ? "(Untitled)" : info.title;
    if (label.length() > 20) {
        label = label.left(18) + "..";
    }
    if (info.modified) {
        label.prepend("* ");
    }
    return label;
}

}  // namespace ui
}  // namespace bastionx
