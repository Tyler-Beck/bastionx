#include "bastionx/ui/ModeSelectorBar.h"
#include "bastionx/ui/UIConstants.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QStyle>

namespace bastionx {
namespace ui {

using namespace constants;

ModeSelectorBar::ModeSelectorBar(QWidget* parent)
    : QWidget(parent), current_(Notes)
{
    setObjectName("modeSelectorBar");
    setFixedHeight(kModeSelectorBarHeight);
    setupUi();
}

void ModeSelectorBar::setupUi() {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);

    // Top section: Horizontal layout with three segments
    auto* segments_layout = new QHBoxLayout();
    segments_layout->setContentsMargins(0, 0, 0, 0);
    segments_layout->setSpacing(0);

    // Create labels for each segment
    auto createLabel = [this](const QString& text) {
        auto* label = new QLabel(text, this);
        label->setAlignment(Qt::AlignCenter);
        label->setObjectName("modeSegmentInactive");
        label->setCursor(Qt::PointingHandCursor);
        return label;
    };

    notes_label_ = createLabel("NOTES");
    search_label_ = createLabel("SEARCH");
    settings_label_ = createLabel("SETTINGS");

    segments_layout->addWidget(notes_label_, 1);
    segments_layout->addWidget(search_label_, 1);
    segments_layout->addWidget(settings_label_, 1);

    main_layout->addLayout(segments_layout, 1);

    // Bottom section: Blade indicator
    blade_indicator_ = new QFrame(this);
    blade_indicator_->setObjectName("bladeIndicator");
    blade_indicator_->setFixedHeight(4);

    // Initially position blade under first segment (will be properly positioned in resizeEvent)
    blade_indicator_->setGeometry(0, kModeSelectorBarHeight - 4, 100, 4);

    updateSegmentStates();
}

void ModeSelectorBar::setActivity(Activity activity) {
    if (current_ != activity) {
        current_ = activity;
        updateSegmentStates();
        animateBladeTo(activity);
        emit activityChanged(activity);
    }
}

void ModeSelectorBar::updateSegmentStates() {
    auto style = [](QLabel* label, bool active) {
        label->setObjectName(active ? "modeSegmentActive" : "modeSegmentInactive");
        // Force style refresh
        label->style()->unpolish(label);
        label->style()->polish(label);
    };

    style(notes_label_, current_ == Notes);
    style(search_label_, current_ == Search);
    style(settings_label_, current_ == Settings);
}

void ModeSelectorBar::animateBladeTo(Activity activity) {
    int segment_width = width() / 3;
    int target_x = static_cast<int>(activity) * segment_width;

    QPropertyAnimation* anim = new QPropertyAnimation(blade_indicator_, "geometry");
    anim->setDuration(200);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(blade_indicator_->geometry());

    QRect target(target_x, kModeSelectorBarHeight - 4, segment_width, 4);
    anim->setEndValue(target);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ModeSelectorBar::mousePressEvent(QMouseEvent* event) {
    int segment_width = width() / 3;
    int x = event->pos().x();

    Activity clicked = Notes;
    if (x < segment_width) {
        clicked = Notes;
    } else if (x < segment_width * 2) {
        clicked = Search;
    } else {
        clicked = Settings;
    }

    setActivity(clicked);
}

void ModeSelectorBar::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    // Recalculate blade position without animation
    int segment_width = width() / 3;
    int x = static_cast<int>(current_) * segment_width;
    blade_indicator_->setGeometry(x, kModeSelectorBarHeight - 4, segment_width, 4);
}

}  // namespace ui
}  // namespace bastionx
