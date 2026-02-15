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
    tab_layout_->setSpacing(0);  // No gaps between tabs (use separators instead)
    tab_layout_->addStretch();

    scroll_area_->setWidget(scroll_content_);
    outer->addWidget(scroll_area_);

    // Create blade indicator (animated bottom indicator)
    blade_indicator_ = new QFrame(this);
    blade_indicator_->setObjectName("tabBladeIndicator");
    blade_indicator_->setFixedHeight(4);
    blade_indicator_->setGeometry(0, kTabBarHeight - 4, 0, 4);  // Start with 0 width
    blade_indicator_->hide();  // Hidden until first tab is added

    // Create reusable animation object
    blade_animation_ = new QPropertyAnimation(blade_indicator_, "geometry", this);
    blade_animation_->setDuration(200);
    blade_animation_->setEasingCurve(QEasingCurve::OutCubic);
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
    container_layout->setSpacing(4);  // 4px gap between title and close button

    // Tab title button
    info.button = new QPushButton(tabLabel(info), info.container);
    info.button->setCursor(Qt::PointingHandCursor);
    info.button->setFlat(true);
    info.button->setMinimumWidth(kTabMinWidth);
    info.button->setMaximumWidth(kTabMaxWidth);
    container_layout->addWidget(info.button);

    // Close button
    info.close_button = new QPushButton("×", info.container);  // Use × symbol (U+00D7)
    info.close_button->setObjectName("tabCloseButton");
    info.close_button->setCursor(Qt::PointingHandCursor);
    info.close_button->setFlat(true);
    info.close_button->setFixedSize(16, 16);  // 16x16 instead of 20x20
    container_layout->addWidget(info.close_button);

    // Insert before the stretch spacer
    int insert_pos = tab_layout_->count() - 1;  // before the stretch

    // Add separator if this is not the first tab
    if (!tabs_.empty()) {
        QFrame* separator = new QFrame(scroll_content_);
        separator->setObjectName("tabSeparator");
        separator->setFixedWidth(1);
        separator->setFixedHeight(18);  // 18px tall, centered in 32px bar

        tab_layout_->insertWidget(insert_pos, separator);
        tabs_.back().right_separator = separator;
        insert_pos++;  // Adjust insert position after adding separator
    }

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

    // Remove and delete the separator if it exists
    if (it->right_separator) {
        tab_layout_->removeWidget(it->right_separator);
        delete it->right_separator;
        it->right_separator = nullptr;
    }

    // Remove the container widget from layout
    tab_layout_->removeWidget(it->container);
    delete it->container;  // Deletes button + close_button too
    tabs_.erase(it);

    // If we removed the active tab, switch to the last tab (or clear)
    if (active_note_id_ == note_id) {
        if (!tabs_.empty()) {
            active_note_id_ = tabs_.back().note_id;
            updateTabStyles();
            updateBladeGeometry(true);  // Animate blade to new active tab
            emit tabSelected(active_note_id_);
        } else {
            active_note_id_ = 0;
            blade_indicator_->hide();  // Hide blade when no tabs
            updateTabStyles();
        }
    } else {
        // Active tab didn't change, but positions may have shifted
        updateBladeGeometry(false);  // Update blade position without animation
    }
}

void TabBar::setActiveTab(int64_t note_id) {
    if (active_note_id_ != note_id) {
        active_note_id_ = note_id;
        updateTabStyles();
        updateBladeGeometry(true);  // Animate blade to new active tab
    }
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
        label.prepend("● ");  // Unicode bullet U+25CF
    }
    return label;
}

void TabBar::updateBladeGeometry(bool animated) {
    // Find the active tab
    auto it = std::find_if(tabs_.begin(), tabs_.end(),
        [this](const TabInfo& t) { return t.note_id == active_note_id_; });

    if (it == tabs_.end()) {
        blade_indicator_->hide();
        return;
    }

    // Calculate blade position
    QRect tab_geom = getTabGeometry(it->note_id);
    if (tab_geom.isNull()) {
        blade_indicator_->hide();
        return;
    }

    QRect target_geom(tab_geom.x(), kTabBarHeight - 4, tab_geom.width(), 4);

    if (animated && blade_indicator_->isVisible()) {
        blade_animation_->stop();
        blade_animation_->setStartValue(blade_indicator_->geometry());
        blade_animation_->setEndValue(target_geom);
        blade_animation_->start();
    } else {
        blade_indicator_->setGeometry(target_geom);
    }

    blade_indicator_->show();
    blade_indicator_->raise();
}

QRect TabBar::getTabGeometry(int64_t note_id) const {
    auto it = std::find_if(tabs_.begin(), tabs_.end(),
        [note_id](const TabInfo& t) { return t.note_id == note_id; });

    if (it == tabs_.end() || !it->container) {
        return QRect();
    }

    // Get container geometry in scroll_content_ coordinates
    QRect container_geom = it->container->geometry();

    // Convert to TabBar coordinates (handle scrolling)
    QPoint viewport_offset = scroll_area_->widget()->pos();
    container_geom.translate(viewport_offset);

    QPoint tabbar_offset = scroll_area_->pos();
    container_geom.translate(tabbar_offset);

    return container_geom;
}

void TabBar::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateBladeGeometry(false);  // Recalculate without animation
}

}  // namespace ui
}  // namespace bastionx
