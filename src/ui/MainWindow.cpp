#include "bastionx/ui/MainWindow.h"
#include "bastionx/ui/UnlockScreen.h"
#include "bastionx/ui/NotesPanel.h"
#include "bastionx/ui/ClipboardGuard.h"
#include "bastionx/ui/SettingsDialog.h"
#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QMessageBox>

namespace bastionx {
namespace ui {

MainWindow::MainWindow(const std::string& vault_path, QWidget* parent)
    : QMainWindow(parent)
{
    vault_ = std::make_unique<vault::VaultService>(vault_path);

    // Toolbar
    setupToolbar();

    // Stacked widget
    stack_ = new QStackedWidget(this);
    setCentralWidget(stack_);

    unlock_screen_ = new UnlockScreen(this);
    notes_panel_ = new NotesPanel(this);
    stack_->addWidget(unlock_screen_);  // index 0
    stack_->addWidget(notes_panel_);    // index 1

    // Signals from UnlockScreen
    connect(unlock_screen_, &UnlockScreen::unlockRequested,
            this, &MainWindow::onUnlockRequested);
    connect(unlock_screen_, &UnlockScreen::createRequested,
            this, &MainWindow::onCreateRequested);

    // Inactivity timer
    inactivity_timer_ = new QTimer(this);
    inactivity_timer_->setSingleShot(true);
    connect(inactivity_timer_, &QTimer::timeout,
            this, &MainWindow::onInactivityTimeout);

    // Clipboard guard
    clipboard_guard_ = new ClipboardGuard(this);

    // Install global event filter for inactivity detection
    qApp->installEventFilter(this);

    // Start on unlock screen
    showUnlockScreen();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupToolbar() {
    toolbar_ = addToolBar("main");
    toolbar_->setMovable(false);
    toolbar_->setFloatable(false);

    title_label_ = new QLabel("BASTIONX", this);
    title_label_->setObjectName("titleLabel");
    toolbar_->addWidget(title_label_);

    // Expanding spacer
    auto* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setStyleSheet("background: transparent;");
    toolbar_->addWidget(spacer);

    settings_button_ = new QPushButton("SETTINGS", this);
    settings_button_->setObjectName("lockButton");  // reuse lockButton style
    connect(settings_button_, &QPushButton::clicked,
            this, &MainWindow::onSettingsRequested);
    toolbar_->addWidget(settings_button_);

    lock_button_ = new QPushButton("LOCK", this);
    lock_button_->setObjectName("lockButton");
    connect(lock_button_, &QPushButton::clicked,
            this, &MainWindow::onLockRequested);
    toolbar_->addWidget(lock_button_);
}

void MainWindow::showUnlockScreen() {
    unlock_screen_->setVaultState(vault_->state());
    unlock_screen_->reset();
    stack_->setCurrentIndex(0);
    lock_button_->hide();
    settings_button_->hide();
    inactivity_timer_->stop();
}

void MainWindow::showNotesPanel() {
    repo_ = std::make_unique<storage::NotesRepository>(vault_->vault_path());
    notes_panel_->loadNotes(repo_.get(), &vault_->notes_subkey());
    stack_->setCurrentIndex(1);
    lock_button_->show();
    settings_button_->show();
    loadAndApplySettings();
    resetInactivityTimer();
}

void MainWindow::loadAndApplySettings() {
    std::string json = vault_->load_settings();
    if (json.empty()) {
        settings_ = vault::VaultSettings::defaults();
    } else {
        settings_ = vault::VaultSettings::from_json(json);
    }

    // Apply auto-lock timeout
    // (resetInactivityTimer will use settings_.auto_lock_minutes)

    // Apply clipboard guard settings
    clipboard_guard_->setEnabled(settings_.clipboard_clear_enabled);
    clipboard_guard_->setClearSeconds(settings_.clipboard_clear_seconds);
}

void MainWindow::onUnlockRequested(const QString& password) {
    unlock_screen_->setSubmitBusy(true);
    QApplication::processEvents();

    bool ok = vault_->unlock(password.toStdString());

    if (ok) {
        showNotesPanel();
    } else {
        unlock_screen_->showError("Wrong password");
        unlock_screen_->setSubmitBusy(false);
    }
}

void MainWindow::onCreateRequested(const QString& password) {
    if (password.isEmpty()) {
        unlock_screen_->showError("Password cannot be empty");
        return;
    }

    unlock_screen_->setSubmitBusy(true);
    QApplication::processEvents();

    bool ok = vault_->create(password.toStdString());

    if (ok) {
        showNotesPanel();
    } else {
        unlock_screen_->showError("Failed to create vault");
        unlock_screen_->setSubmitBusy(false);
    }
}

void MainWindow::onLockRequested() {
    // Clear clipboard if we own it
    clipboard_guard_->clearNow();

    notes_panel_->prepareForLock();
    repo_.reset();
    vault_->lock();
    showUnlockScreen();
}

void MainWindow::onInactivityTimeout() {
    if (vault_ && vault_->is_unlocked()) {
        onLockRequested();
    }
}

void MainWindow::onSettingsRequested() {
    auto* dialog = new SettingsDialog(settings_, this);

    connect(dialog, &SettingsDialog::settingsChanged,
            this, &MainWindow::onSettingsChanged);
    connect(dialog, &SettingsDialog::passwordChangeRequested,
            this, &MainWindow::onPasswordChangeRequested);

    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::onSettingsChanged(const vault::VaultSettings& settings) {
    settings_ = settings;
    vault_->save_settings(settings_.to_json());

    // Apply immediately
    clipboard_guard_->setEnabled(settings_.clipboard_clear_enabled);
    clipboard_guard_->setClearSeconds(settings_.clipboard_clear_seconds);
    resetInactivityTimer();
}

void MainWindow::onPasswordChangeRequested(const QString& current_pw,
                                           const QString& new_pw) {
    // Close repo before password change (re-encryption needs exclusive DB access)
    notes_panel_->prepareForLock();
    repo_.reset();

    QApplication::processEvents();

    bool ok = vault_->change_password(current_pw.toStdString(), new_pw.toStdString());

    if (ok) {
        QMessageBox::information(this, "Password Changed",
                                 "Your master password has been changed successfully.");
        // Reopen repo with new subkey
        repo_ = std::make_unique<storage::NotesRepository>(vault_->vault_path());
        notes_panel_->loadNotes(repo_.get(), &vault_->notes_subkey());
    } else {
        QMessageBox::warning(this, "Password Change Failed",
                             "Current password is incorrect.");
        // Reopen repo with existing subkey
        repo_ = std::make_unique<storage::NotesRepository>(vault_->vault_path());
        notes_panel_->loadNotes(repo_.get(), &vault_->notes_subkey());
    }
}

void MainWindow::resetInactivityTimer() {
    if (vault_ && vault_->is_unlocked()) {
        int timeout_ms = settings_.auto_lock_minutes * 60 * 1000;
        inactivity_timer_->start(timeout_ms);
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::KeyPress:
        case QEvent::Wheel:
            resetInactivityTimer();
            break;
        default:
            break;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (vault_ && vault_->is_unlocked()) {
        clipboard_guard_->clearNow();
        notes_panel_->prepareForLock();
        repo_.reset();
        vault_->lock();
    }
    event->accept();
}

}  // namespace ui
}  // namespace bastionx
