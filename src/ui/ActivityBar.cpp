#include "bastionx/ui/ActivityBar.h"
#include "bastionx/ui/UIConstants.h"
#include <QStyle>

namespace bastionx {
namespace ui {

using namespace constants;

ActivityBar::ActivityBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("activityBar");
    setFixedWidth(kActivityBarWidth);
    setupUi();
}

void ActivityBar::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, kMarginSmall, 0, kMarginSmall);
    layout->setSpacing(kSpacingTight);  // 4px gap between buttons

    auto makeButton = [this](const QString& text) {
        auto* btn = new QPushButton(text, this);
        btn->setFixedSize(kActivityButtonSize, kActivityButtonSize);
        btn->setFlat(true);
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };

    notes_btn_ = makeButton("N");
    search_btn_ = makeButton("S");
    settings_btn_ = makeButton("G");

    notes_btn_->setToolTip("Notes");
    search_btn_->setToolTip("Search");
    settings_btn_->setToolTip("Settings");

    layout->addWidget(notes_btn_);
    layout->addWidget(search_btn_);
    layout->addStretch();
    layout->addWidget(settings_btn_);

    connect(notes_btn_, &QPushButton::clicked, this, [this] {
        setActivity(Notes);
    });
    connect(search_btn_, &QPushButton::clicked, this, [this] {
        setActivity(Search);
    });
    connect(settings_btn_, &QPushButton::clicked, this, [this] {
        setActivity(Settings);
    });

    updateButtonStates();
}

void ActivityBar::setActivity(Activity activity) {
    if (current_ != activity) {
        current_ = activity;
        updateButtonStates();
        emit activityChanged(activity);
    }
}

void ActivityBar::updateButtonStates() {
    auto style = [](QPushButton* btn, bool active) {
        btn->setObjectName(active ? "activityButtonActive" : "activityButton");
        // Force style refresh
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    };

    style(notes_btn_, current_ == Notes);
    style(search_btn_, current_ == Search);
    style(settings_btn_, current_ == Settings);
}

}  // namespace ui
}  // namespace bastionx
