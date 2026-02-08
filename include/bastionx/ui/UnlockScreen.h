#ifndef BASTIONX_UI_UNLOCKSCREEN_H
#define BASTIONX_UI_UNLOCKSCREEN_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "bastionx/vault/VaultService.h"

namespace bastionx {
namespace ui {

class UnlockScreen : public QWidget {
    Q_OBJECT

public:
    explicit UnlockScreen(QWidget* parent = nullptr);

    void setVaultState(vault::VaultState state);
    void reset();
    void setSubmitBusy(bool busy);

signals:
    void unlockRequested(const QString& password);
    void createRequested(const QString& password);

public slots:
    void showError(const QString& message);

private slots:
    void onSubmit();

private:
    void setupUi();

    QLabel*      title_label_ = nullptr;
    QLabel*      status_label_ = nullptr;
    QLineEdit*   password_input_ = nullptr;
    QPushButton* submit_button_ = nullptr;
    QLabel*      error_label_ = nullptr;

    vault::VaultState current_state_ = vault::VaultState::kLocked;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_UNLOCKSCREEN_H
