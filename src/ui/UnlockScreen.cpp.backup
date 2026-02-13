#include "bastionx/ui/UnlockScreen.h"
#include <QVBoxLayout>
#include <QKeyEvent>

namespace bastionx {
namespace ui {

UnlockScreen::UnlockScreen(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void UnlockScreen::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);

    auto* form = new QWidget(this);
    form->setMaximumWidth(400);
    auto* layout = new QVBoxLayout(form);
    layout->setSpacing(16);

    title_label_ = new QLabel("BASTIONX", form);
    title_label_->setObjectName("titleLabel");
    title_label_->setAlignment(Qt::AlignCenter);
    title_label_->setStyleSheet("color: #5f8a6e; font-size: 28px; font-weight: bold;");
    layout->addWidget(title_label_);

    layout->addSpacing(20);

    status_label_ = new QLabel("Enter master password", form);
    status_label_->setAlignment(Qt::AlignCenter);
    status_label_->setStyleSheet("color: #808080; font-size: 13px;");
    layout->addWidget(status_label_);

    password_input_ = new QLineEdit(form);
    password_input_->setObjectName("passwordInput");
    password_input_->setEchoMode(QLineEdit::Password);
    password_input_->setPlaceholderText("Password");
    layout->addWidget(password_input_);

    submit_button_ = new QPushButton("UNLOCK", form);
    submit_button_->setObjectName("submitButton");
    layout->addWidget(submit_button_);

    error_label_ = new QLabel("", form);
    error_label_->setObjectName("errorLabel");
    error_label_->setAlignment(Qt::AlignCenter);
    error_label_->hide();
    layout->addWidget(error_label_);

    outer->addWidget(form);

    connect(submit_button_, &QPushButton::clicked, this, &UnlockScreen::onSubmit);
    connect(password_input_, &QLineEdit::returnPressed, this, &UnlockScreen::onSubmit);
}

void UnlockScreen::setVaultState(vault::VaultState state) {
    current_state_ = state;
    if (state == vault::VaultState::kNoVault) {
        status_label_->setText("Create a new vault");
        submit_button_->setText("CREATE VAULT");
    } else {
        status_label_->setText("Enter master password");
        submit_button_->setText("UNLOCK");
    }
}

void UnlockScreen::reset() {
    password_input_->clear();
    error_label_->hide();
    error_label_->clear();
    submit_button_->setEnabled(true);
    password_input_->setFocus();
}

void UnlockScreen::showError(const QString& message) {
    error_label_->setText(message);
    error_label_->show();
}

void UnlockScreen::setSubmitBusy(bool busy) {
    submit_button_->setEnabled(!busy);
    if (busy) {
        if (current_state_ == vault::VaultState::kNoVault) {
            submit_button_->setText("CREATING...");
        } else {
            submit_button_->setText("UNLOCKING...");
        }
    } else {
        if (current_state_ == vault::VaultState::kNoVault) {
            submit_button_->setText("CREATE VAULT");
        } else {
            submit_button_->setText("UNLOCK");
        }
    }
}

void UnlockScreen::onSubmit() {
    error_label_->hide();
    QString password = password_input_->text();

    if (current_state_ == vault::VaultState::kNoVault) {
        emit createRequested(password);
    } else {
        emit unlockRequested(password);
    }
}

}  // namespace ui
}  // namespace bastionx
