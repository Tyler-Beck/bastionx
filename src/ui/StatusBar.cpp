#include "bastionx/ui/StatusBar.h"
#include "bastionx/ui/UIConstants.h"

namespace bastionx {
namespace ui {

using namespace constants;

StatusBar::StatusBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("statusBar");
    setFixedHeight(kStatusBarHeight);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(kMarginSmall, 0, kMarginSmall, 0);
    layout->setSpacing(kMarginSmall);  // 8px between status items

    save_label_ = new QLabel("", this);
    save_label_->setProperty("class", "statusItem");
    layout->addWidget(save_label_);

    encryption_label_ = new QLabel("", this);
    encryption_label_->setObjectName("encryptionIndicator");
    layout->addWidget(encryption_label_);

    layout->addStretch();

    word_count_label_ = new QLabel("", this);
    word_count_label_->setProperty("class", "statusItem");
    layout->addWidget(word_count_label_);
}

void StatusBar::setSaveState(const QString& state) {
    save_label_->setText(state);
}

void StatusBar::setWordCount(int words, int chars) {
    word_count_label_->setText(
        QString("Words: %1  Chars: %2").arg(words).arg(chars));
}

void StatusBar::setEncryptionIndicator(bool encrypted) {
    encryption_label_->setText(encrypted ? "Encrypted" : "");
}

void StatusBar::clear() {
    save_label_->clear();
    encryption_label_->clear();
    word_count_label_->clear();
}

}  // namespace ui
}  // namespace bastionx
