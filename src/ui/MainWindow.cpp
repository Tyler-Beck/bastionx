#include "bastionx/ui/MainWindow.h"
#include "bastionx/ui/UnlockScreen.h"
#include "bastionx/ui/NotesPanel.h"
#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>

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
    inactivity_timer_->stop();
}

void MainWindow::showNotesPanel() {
    repo_ = std::make_unique<storage::NotesRepository>(vault_->vault_path());
    notes_panel_->loadNotes(repo_.get(), &vault_->notes_subkey());
    stack_->setCurrentIndex(1);
    lock_button_->show();
    resetInactivityTimer();
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

void MainWindow::resetInactivityTimer() {
    if (vault_ && vault_->is_unlocked()) {
        inactivity_timer_->start(kDefaultTimeoutMs);
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
        notes_panel_->prepareForLock();
        repo_.reset();
        vault_->lock();
    }
    event->accept();
}

}  // namespace ui
}  // namespace bastionx
