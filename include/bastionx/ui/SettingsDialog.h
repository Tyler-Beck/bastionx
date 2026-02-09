#ifndef BASTIONX_UI_SETTINGSDIALOG_H
#define BASTIONX_UI_SETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "bastionx/vault/VaultSettings.h"

namespace bastionx {
namespace ui {

/**
 * @brief Modal settings dialog
 *
 * Three sections: Auto-Lock, Clipboard, Password Change.
 * Emits settingsChanged when Save is clicked.
 * Emits passwordChangeRequested when Change Password is clicked.
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(const vault::VaultSettings& current, QWidget* parent = nullptr);

    vault::VaultSettings currentSettings() const;

signals:
    void settingsChanged(const vault::VaultSettings& settings);
    void passwordChangeRequested(const QString& current_pw,
                                 const QString& new_pw);

private slots:
    void onSave();
    void onChangePassword();
    void onPasswordFieldsChanged();

private:
    void setupUi(const vault::VaultSettings& current);

    // Auto-Lock
    QSpinBox* auto_lock_spin_ = nullptr;

    // Clipboard
    QCheckBox* clipboard_enabled_ = nullptr;
    QSpinBox* clipboard_seconds_spin_ = nullptr;

    // Password change
    QLineEdit* current_pw_ = nullptr;
    QLineEdit* new_pw_ = nullptr;
    QLineEdit* confirm_pw_ = nullptr;
    QPushButton* change_pw_button_ = nullptr;
    QLabel* pw_error_label_ = nullptr;

    // Actions
    QPushButton* save_button_ = nullptr;
    QPushButton* cancel_button_ = nullptr;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_SETTINGSDIALOG_H
