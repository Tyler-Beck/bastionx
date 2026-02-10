#ifndef BASTIONX_UI_MAINWINDOW_H
#define BASTIONX_UI_MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QToolBar>
#include <memory>
#include "bastionx/vault/VaultService.h"
#include "bastionx/vault/VaultSettings.h"
#include "bastionx/storage/NotesRepository.h"

namespace bastionx {
namespace ui {

class UnlockScreen;
class NotesPanel;
class ClipboardGuard;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const std::string& vault_path, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onUnlockRequested(const QString& password);
    void onCreateRequested(const QString& password);
    void onLockRequested();
    void onInactivityTimeout();
    void onSettingsRequested();
    void onSettingsChanged(const vault::VaultSettings& settings);
    void onPasswordChangeRequested(const QString& current_pw,
                                   const QString& new_pw);

private:
    void showUnlockScreen();
    void showNotesPanel();
    void resetInactivityTimer();
    void setupToolbar();
    void loadAndApplySettings();

    // UI
    QStackedWidget* stack_ = nullptr;
    UnlockScreen*   unlock_screen_ = nullptr;
    NotesPanel*     notes_panel_ = nullptr;
    QToolBar*       toolbar_ = nullptr;
    QPushButton*    lock_button_ = nullptr;
    QLabel*         title_label_ = nullptr;

    // Backend
    std::unique_ptr<vault::VaultService>      vault_;
    std::unique_ptr<storage::NotesRepository> repo_;

    // Settings & Clipboard
    vault::VaultSettings settings_;
    ClipboardGuard* clipboard_guard_ = nullptr;

    // Inactivity
    QTimer* inactivity_timer_ = nullptr;
    static constexpr int kDefaultTimeoutMs = 5 * 60 * 1000;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_MAINWINDOW_H
