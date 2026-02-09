#include "bastionx/ui/SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFormLayout>

namespace bastionx {
namespace ui {

SettingsDialog::SettingsDialog(const vault::VaultSettings& current, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    setMinimumWidth(420);
    setModal(true);
    setupUi(current);
}

vault::VaultSettings SettingsDialog::currentSettings() const {
    vault::VaultSettings s;
    s.auto_lock_minutes = auto_lock_spin_->value();
    s.clipboard_clear_enabled = clipboard_enabled_->isChecked();
    s.clipboard_clear_seconds = clipboard_seconds_spin_->value();
    return s;
}

void SettingsDialog::setupUi(const vault::VaultSettings& current) {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(16);

    // === Auto-Lock Group ===
    auto* lock_group = new QGroupBox("Auto-Lock", this);
    auto* lock_layout = new QFormLayout(lock_group);

    auto_lock_spin_ = new QSpinBox(lock_group);
    auto_lock_spin_->setRange(1, 60);
    auto_lock_spin_->setSuffix(" min");
    auto_lock_spin_->setValue(current.auto_lock_minutes);
    lock_layout->addRow("Lock after inactivity:", auto_lock_spin_);

    main_layout->addWidget(lock_group);

    // === Clipboard Group ===
    auto* clip_group = new QGroupBox("Clipboard", this);
    auto* clip_layout = new QFormLayout(clip_group);

    clipboard_enabled_ = new QCheckBox("Auto-clear clipboard", clip_group);
    clipboard_enabled_->setChecked(current.clipboard_clear_enabled);
    clip_layout->addRow(clipboard_enabled_);

    clipboard_seconds_spin_ = new QSpinBox(clip_group);
    clipboard_seconds_spin_->setRange(10, 120);
    clipboard_seconds_spin_->setSuffix(" sec");
    clipboard_seconds_spin_->setValue(current.clipboard_clear_seconds);
    clip_layout->addRow("Clear after:", clipboard_seconds_spin_);

    connect(clipboard_enabled_, &QCheckBox::toggled,
            clipboard_seconds_spin_, &QSpinBox::setEnabled);
    clipboard_seconds_spin_->setEnabled(current.clipboard_clear_enabled);

    main_layout->addWidget(clip_group);

    // === Password Change Group ===
    auto* pw_group = new QGroupBox("Change Password", this);
    auto* pw_layout = new QFormLayout(pw_group);

    current_pw_ = new QLineEdit(pw_group);
    current_pw_->setEchoMode(QLineEdit::Password);
    current_pw_->setPlaceholderText("Current password");
    pw_layout->addRow("Current:", current_pw_);

    new_pw_ = new QLineEdit(pw_group);
    new_pw_->setEchoMode(QLineEdit::Password);
    new_pw_->setPlaceholderText("New password");
    pw_layout->addRow("New:", new_pw_);

    confirm_pw_ = new QLineEdit(pw_group);
    confirm_pw_->setEchoMode(QLineEdit::Password);
    confirm_pw_->setPlaceholderText("Confirm new password");
    pw_layout->addRow("Confirm:", confirm_pw_);

    change_pw_button_ = new QPushButton("CHANGE PASSWORD", pw_group);
    change_pw_button_->setObjectName("changePasswordButton");
    change_pw_button_->setEnabled(false);
    pw_layout->addRow(change_pw_button_);

    pw_error_label_ = new QLabel("", pw_group);
    pw_error_label_->setObjectName("errorLabel");
    pw_error_label_->setAlignment(Qt::AlignCenter);
    pw_error_label_->hide();
    pw_layout->addRow(pw_error_label_);

    connect(current_pw_, &QLineEdit::textChanged, this, &SettingsDialog::onPasswordFieldsChanged);
    connect(new_pw_, &QLineEdit::textChanged, this, &SettingsDialog::onPasswordFieldsChanged);
    connect(confirm_pw_, &QLineEdit::textChanged, this, &SettingsDialog::onPasswordFieldsChanged);
    connect(change_pw_button_, &QPushButton::clicked, this, &SettingsDialog::onChangePassword);

    main_layout->addWidget(pw_group);

    // === Bottom buttons ===
    auto* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    cancel_button_ = new QPushButton("CANCEL", this);
    cancel_button_->setObjectName("cancelButton");
    connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);
    button_layout->addWidget(cancel_button_);

    save_button_ = new QPushButton("SAVE", this);
    save_button_->setObjectName("submitButton");
    connect(save_button_, &QPushButton::clicked, this, &SettingsDialog::onSave);
    button_layout->addWidget(save_button_);

    main_layout->addLayout(button_layout);
}

void SettingsDialog::onSave() {
    emit settingsChanged(currentSettings());
    accept();
}

void SettingsDialog::onChangePassword() {
    pw_error_label_->hide();

    if (new_pw_->text() != confirm_pw_->text()) {
        pw_error_label_->setText("Passwords do not match");
        pw_error_label_->show();
        return;
    }

    emit passwordChangeRequested(current_pw_->text(), new_pw_->text());
}

void SettingsDialog::onPasswordFieldsChanged() {
    bool all_filled = !current_pw_->text().isEmpty() &&
                      !new_pw_->text().isEmpty() &&
                      !confirm_pw_->text().isEmpty();
    bool match = new_pw_->text() == confirm_pw_->text();
    change_pw_button_->setEnabled(all_filled && match);

    // Show mismatch hint if both fields have content but don't match
    if (!new_pw_->text().isEmpty() && !confirm_pw_->text().isEmpty() && !match) {
        pw_error_label_->setText("Passwords do not match");
        pw_error_label_->show();
    } else {
        pw_error_label_->hide();
    }
}

}  // namespace ui
}  // namespace bastionx
