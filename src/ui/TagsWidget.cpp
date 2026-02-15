#include "bastionx/ui/TagsWidget.h"
#include "bastionx/ui/UIConstants.h"
#include <algorithm>

namespace bastionx {
namespace ui {

using namespace constants;

TagsWidget::TagsWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("tagsWidget");
    setFixedHeight(kTagsWidgetHeight);

    auto* main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(kMarginSmall, kMarginTiny, kMarginSmall, kMarginTiny);
    main_layout->setSpacing(0);

    // Label for context
    tags_label_ = new QLabel("Tags:", this);
    tags_label_->setObjectName("tagsLabel");
    main_layout->addWidget(tags_label_);
    main_layout->addSpacing(kSpacingTight);

    // Chip layout for tag chips
    chip_layout_ = new QHBoxLayout();
    chip_layout_->setSpacing(kSpacingTight);
    main_layout->addLayout(chip_layout_);

    main_layout->addStretch();

    // Input for adding new tags
    add_input_ = new QLineEdit(this);
    add_input_->setObjectName("tagInput");
    add_input_->setPlaceholderText("+ add tag");
    add_input_->setFixedWidth(kTagInputWidth);
    add_input_->setFixedHeight(kButtonHeightCompact);
    main_layout->addWidget(add_input_);

    connect(add_input_, &QLineEdit::returnPressed, this, &TagsWidget::onAddTag);
}

void TagsWidget::setTags(const std::vector<std::string>& tags) {
    tags_ = tags;
    rebuildChips();
}

std::vector<std::string> TagsWidget::tags() const {
    return tags_;
}

void TagsWidget::clear() {
    tags_.clear();
    rebuildChips();
}

void TagsWidget::onAddTag() {
    QString text = add_input_->text().trimmed().toLower();
    if (text.isEmpty()) return;

    std::string tag = text.toStdString();

    // No duplicates
    if (std::find(tags_.begin(), tags_.end(), tag) != tags_.end()) {
        add_input_->clear();
        return;
    }

    tags_.push_back(tag);
    add_input_->clear();
    rebuildChips();
    emit tagsChanged();
}

void TagsWidget::onRemoveTag() {
    auto* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    std::string tag = btn->property("tagName").toString().toStdString();
    tags_.erase(std::remove(tags_.begin(), tags_.end(), tag), tags_.end());
    rebuildChips();
    emit tagsChanged();
}

void TagsWidget::rebuildChips() {
    // Remove all chip buttons (keep stretch + add_input_)
    while (chip_layout_->count() > 2) {
        auto* item = chip_layout_->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Insert chip buttons before the stretch
    int insert_pos = 0;
    for (const auto& tag : tags_) {
        auto* chip = new QPushButton(QString::fromStdString(tag) + " x", this);
        chip->setObjectName("tagChip");
        chip->setFixedHeight(kButtonHeightCompact);
        chip->setProperty("tagName", QString::fromStdString(tag));
        connect(chip, &QPushButton::clicked, this, &TagsWidget::onRemoveTag);
        chip_layout_->insertWidget(insert_pos++, chip);
    }
}

}  // namespace ui
}  // namespace bastionx
