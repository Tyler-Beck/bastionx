#ifndef BASTIONX_UI_STYLESHEET_H
#define BASTIONX_UI_STYLESHEET_H

namespace bastionx {
namespace ui {

inline const char* const kStyleSheet = R"QSS(

/* === Global === */
QWidget {
    background-color: #1a1a1a;
    color: #d4d4d4;
    font-family: "Fira Mono", "Consolas", "JetBrains Mono", monospace;
    font-size: 13px;
}

QMainWindow {
    background-color: #1a1a1a;
}

/* === Toolbar === */
QToolBar {
    background-color: #1a1a1a;
    border-bottom: 1px solid #3a3a3a;
    padding: 6px 12px;
    spacing: 8px;
}

QToolBar QLabel#titleLabel {
    color: #5f8a6e;
    font-size: 16px;
    font-weight: bold;
}

/* === Buttons === */
QPushButton {
    background-color: #252525;
    color: #d4d4d4;
    border: 1px solid #3a3a3a;
    padding: 8px 16px;
    font-size: 13px;
}

QPushButton:hover {
    background-color: #303030;
    border-color: #5f8a6e;
}

QPushButton:pressed {
    background-color: #1a1a1a;
}

QPushButton:disabled {
    color: #505050;
    border-color: #2a2a2a;
    background-color: #1e1e1e;
}

QPushButton#submitButton {
    background-color: #2a3a2e;
    border-color: #5f8a6e;
    color: #5f8a6e;
    font-size: 14px;
    padding: 10px 24px;
}

QPushButton#submitButton:hover {
    background-color: #354a3a;
    color: #6ea07f;
}

QPushButton#submitButton:disabled {
    background-color: #1e1e1e;
    border-color: #2a2a2a;
    color: #505050;
}

QPushButton#lockButton {
    background-color: transparent;
    border: 1px solid #3a3a3a;
    color: #808080;
    padding: 4px 12px;
}

QPushButton#lockButton:hover {
    border-color: #a04040;
    color: #a04040;
}

QPushButton#newNoteButton {
    background-color: #252525;
    border: 1px solid #3a3a3a;
    color: #5f8a6e;
    padding: 6px;
    text-align: left;
}

QPushButton#newNoteButton:hover {
    background-color: #303030;
    border-color: #5f8a6e;
}

QPushButton#deleteButton {
    background-color: transparent;
    border: 1px solid #3a3a3a;
    color: #a04040;
    padding: 4px 12px;
}

QPushButton#deleteButton:hover {
    border-color: #a04040;
    background-color: #2a1a1a;
}

/* === Line Edits === */
QLineEdit {
    background-color: #252525;
    color: #d4d4d4;
    border: 1px solid #3a3a3a;
    padding: 8px;
    selection-background-color: #3a5a42;
}

QLineEdit:focus {
    border-color: #5f8a6e;
}

QLineEdit#passwordInput {
    font-size: 16px;
    padding: 10px;
}

QLineEdit#titleInput {
    font-size: 18px;
    font-weight: bold;
    border: none;
    border-bottom: 1px solid #3a3a3a;
    background-color: #1a1a1a;
    padding: 8px 4px;
}

QLineEdit#titleInput:focus {
    border-bottom-color: #5f8a6e;
}

/* === Plain Text Edit === */
QPlainTextEdit {
    background-color: #1a1a1a;
    color: #d4d4d4;
    border: none;
    padding: 8px;
    selection-background-color: #3a5a42;
    font-size: 14px;
}

/* === List Widget === */
QListWidget {
    background-color: #1a1a1a;
    border: none;
    outline: none;
}

QListWidget::item {
    padding: 10px 8px;
    border-bottom: 1px solid #252525;
    color: #d4d4d4;
}

QListWidget::item:selected {
    background-color: #252525;
    color: #5f8a6e;
    border-left: 2px solid #5f8a6e;
}

QListWidget::item:hover:!selected {
    background-color: #202020;
}

/* === Splitter === */
QSplitter::handle {
    background-color: #3a3a3a;
    width: 1px;
}

/* === Scrollbars === */
QScrollBar:vertical {
    background-color: #1a1a1a;
    width: 8px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background-color: #3a3a3a;
    min-height: 30px;
    border-radius: 4px;
}

QScrollBar::handle:vertical:hover {
    background-color: #505050;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    height: 0;
}

/* === Labels === */
QLabel#errorLabel {
    color: #a04040;
    font-size: 12px;
}

QLabel#statusLabel {
    color: #808080;
    font-size: 11px;
}

/* === Message Box (delete confirmation) === */
QMessageBox {
    background-color: #1a1a1a;
}

QMessageBox QLabel {
    color: #d4d4d4;
}

QMessageBox QPushButton {
    min-width: 80px;
}

/* === Dialog === */
QDialog {
    background-color: #1a1a1a;
}

/* === Group Box === */
QGroupBox {
    border: 1px solid #3a3a3a;
    margin-top: 12px;
    padding: 16px 12px 8px 12px;
    font-size: 13px;
    font-weight: bold;
    color: #5f8a6e;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 4px;
}

/* === Spin Box === */
QSpinBox {
    background-color: #252525;
    color: #d4d4d4;
    border: 1px solid #3a3a3a;
    padding: 6px 8px;
    min-width: 80px;
}

QSpinBox:focus {
    border-color: #5f8a6e;
}

QSpinBox::up-button, QSpinBox::down-button {
    background-color: #303030;
    border: none;
    width: 16px;
}

QSpinBox::up-button:hover, QSpinBox::down-button:hover {
    background-color: #3a3a3a;
}

/* === Check Box === */
QCheckBox {
    color: #d4d4d4;
    spacing: 8px;
}

QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid #3a3a3a;
    background-color: #252525;
}

QCheckBox::indicator:checked {
    background-color: #5f8a6e;
    border-color: #5f8a6e;
}

QCheckBox::indicator:hover {
    border-color: #5f8a6e;
}

/* === Change Password Button === */
QPushButton#changePasswordButton {
    background-color: #2a2a3a;
    border-color: #5a5a8a;
    color: #8a8abf;
    padding: 8px 16px;
}

QPushButton#changePasswordButton:hover {
    background-color: #35354a;
    color: #9a9ad0;
}

QPushButton#changePasswordButton:disabled {
    background-color: #1e1e1e;
    border-color: #2a2a2a;
    color: #505050;
}

/* === Cancel Button === */
QPushButton#cancelButton {
    background-color: transparent;
    border: 1px solid #3a3a3a;
    color: #808080;
}

QPushButton#cancelButton:hover {
    border-color: #505050;
    color: #a0a0a0;
}

/* === Activity Bar === */
QWidget#activityBar {
    background-color: #1e1e1e;
    border-right: 1px solid #3a3a3a;
}

QPushButton#activityButton {
    background-color: transparent;
    color: #808080;
    border: none;
    border-left: 2px solid transparent;
    font-size: 16px;
    font-weight: bold;
    padding: 0;
}

QPushButton#activityButton:hover {
    color: #d4d4d4;
}

QPushButton#activityButtonActive {
    background-color: transparent;
    color: #d4d4d4;
    border: none;
    border-left: 2px solid #5f8a6e;
    font-size: 16px;
    font-weight: bold;
    padding: 0;
}

/* === Sidebar === */
QWidget#sidebar {
    background-color: #1e1e1e;
    border-right: 1px solid #3a3a3a;
}

/* === Filter Input === */
QLineEdit#filterInput {
    background-color: #252525;
    border: 1px solid #3a3a3a;
    border-radius: 0;
    padding: 6px 8px;
    margin: 4px;
    font-size: 12px;
}

QLineEdit#filterInput:focus {
    border-color: #5f8a6e;
}

/* === Tab Bar === */
QWidget#tabBar {
    background-color: #1e1e1e;
    border-bottom: 1px solid #3a3a3a;
}

QScrollArea#tabScrollArea {
    background-color: #1e1e1e;
    border: none;
}

QWidget#tabContainer {
    background-color: transparent;
}

QPushButton#tab {
    background-color: #1e1e1e;
    color: #808080;
    border: none;
    border-bottom: 2px solid transparent;
    padding: 6px 12px;
    font-size: 12px;
    text-align: left;
}

QPushButton#tab:hover {
    color: #d4d4d4;
    background-color: #252525;
}

QPushButton#tabActive {
    background-color: #1a1a1a;
    color: #d4d4d4;
    border: none;
    border-bottom: 2px solid #5f8a6e;
    padding: 6px 12px;
    font-size: 12px;
    text-align: left;
}

QPushButton#tabCloseButton {
    background-color: transparent;
    color: #808080;
    border: none;
    padding: 0;
    font-size: 12px;
    min-width: 16px;
    max-width: 16px;
    min-height: 16px;
    max-height: 16px;
}

QPushButton#tabCloseButton:hover {
    color: #d4d4d4;
    background-color: #3a3a3a;
}

/* === Status Bar === */
QWidget#statusBar {
    background-color: #1e1e1e;
    border-top: 1px solid #3a3a3a;
}

QLabel#encryptionIndicator {
    color: #5f8a6e;
    font-size: 11px;
    padding-left: 8px;
}

QWidget#statusBar QLabel[class="statusItem"] {
    color: #808080;
    font-size: 11px;
}

)QSS";

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_STYLESHEET_H
