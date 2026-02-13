#ifndef BASTIONX_UI_STYLESHEET_H
#define BASTIONX_UI_STYLESHEET_H

namespace bastionx {
namespace ui {

// Phase 7: Premium Dark UI
// Surface levels: #0d0f14 < #13151c < #1a1d26 < #22262f < #2c3040
// Accent green: #4ade80 / #22c55e / #0f2318 (tint)
// Text: #e8eaf0 / #a0a4b8 / #606478

inline const char* const kStyleSheet = R"QSS(

/* ============================================================
   GLOBAL
   ============================================================ */
QWidget {
    background-color: #0d0f14;
    color: #e8eaf0;
    font-family: "Segoe UI", "SF Pro Display", system-ui, sans-serif;
    font-size: 13px;
}

QMainWindow {
    background-color: #0d0f14;
}

/* ============================================================
   TOOLBAR
   ============================================================ */
QToolBar {
    background-color: #0d0f14;
    border-bottom: 1px solid #1e2130;
    padding: 6px 12px;
    spacing: 8px;
}

QToolBar QLabel#titleLabel {
    color: #4ade80;
    font-size: 15px;
    font-weight: bold;
    letter-spacing: 4px;
}

/* ============================================================
   BUTTONS — Base
   ============================================================ */
QPushButton {
    background-color: #22262f;
    color: #e8eaf0;
    border: 1px solid #282c3a;
    border-radius: 4px;
    padding: 8px 16px;
    font-size: 13px;
    font-family: "Segoe UI", sans-serif;
}

QPushButton:hover {
    background-color: #2c3040;
    border-color: #353a4c;
}

QPushButton:pressed {
    background-color: #13151c;
}

QPushButton:disabled {
    color: #606478;
    border-color: #1e2130;
    background-color: #13151c;
}

/* Submit / Primary Button */
QPushButton#submitButton {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #166534, stop:1 #14532d);
    border: 1px solid #22c55e;
    color: #4ade80;
    font-size: 14px;
    font-weight: bold;
    padding: 12px 28px;
    border-radius: 6px;
    letter-spacing: 2px;
}

QPushButton#submitButton:hover {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #1a7a40, stop:1 #166534);
    color: #86efac;
}

QPushButton#submitButton:disabled {
    background-color: #13151c;
    border-color: #1e2130;
    color: #606478;
}

/* Lock Button */
QPushButton#lockButton {
    background-color: transparent;
    border: 1px solid #282c3a;
    border-radius: 4px;
    color: #a0a4b8;
    padding: 4px 14px;
}

QPushButton#lockButton:hover {
    border-color: #f87171;
    color: #f87171;
    background-color: #2a1218;
}

/* New Note Button */
QPushButton#newNoteButton {
    background-color: #13151c;
    border: 1px solid #282c3a;
    border-radius: 4px;
    color: #4ade80;
    padding: 8px 10px;
    text-align: left;
    font-weight: 600;
}

QPushButton#newNoteButton:hover {
    background-color: #1a1d26;
    border-color: #22c55e;
}

/* Delete Button */
QPushButton#deleteButton {
    background-color: transparent;
    border: 1px solid #282c3a;
    border-radius: 4px;
    color: #f87171;
    padding: 4px 14px;
    font-size: 12px;
}

QPushButton#deleteButton:hover {
    border-color: #f87171;
    background-color: #2a1218;
}

/* ============================================================
   LINE EDITS
   ============================================================ */
QLineEdit {
    background-color: #1a1d26;
    color: #e8eaf0;
    border: 1px solid #282c3a;
    border-radius: 4px;
    padding: 8px 10px;
    selection-background-color: #1a3d2e;
    font-family: "Segoe UI", sans-serif;
}

QLineEdit:focus {
    border-color: #4ade80;
}

QLineEdit:disabled {
    color: #606478;
    background-color: #13151c;
}

QLineEdit#passwordInput {
    font-size: 16px;
    padding: 12px 14px;
    border-radius: 6px;
}

QLineEdit#titleInput {
    font-size: 20px;
    font-weight: 600;
    border: none;
    border-bottom: 2px solid #1e2130;
    border-radius: 0;
    background-color: #0d0f14;
    padding: 12px 4px;
    color: #e8eaf0;
}

QLineEdit#titleInput:focus {
    border-bottom-color: #4ade80;
}

/* ============================================================
   RICH TEXT EDITOR
   ============================================================ */
QTextEdit {
    background-color: #0d0f14;
    color: #e8eaf0;
    border: none;
    padding: 12px;
    selection-background-color: #1a3d2e;
    font-family: "Segoe UI", "SF Pro Text", sans-serif;
    font-size: 14px;
}

/* ============================================================
   PLAIN TEXT EDIT (legacy fallback)
   ============================================================ */
QPlainTextEdit {
    background-color: #0d0f14;
    color: #e8eaf0;
    border: none;
    padding: 12px;
    selection-background-color: #1a3d2e;
    font-size: 14px;
}

/* ============================================================
   LIST WIDGET
   ============================================================ */
QListWidget {
    background-color: #13151c;
    border: none;
    outline: none;
}

QListWidget::item {
    padding: 10px 10px;
    border-bottom: 1px solid #1e2130;
    border-left: 3px solid transparent;
    color: #e8eaf0;
}

QListWidget::item:selected {
    background-color: #0f2318;
    color: #4ade80;
    border-left: 3px solid #4ade80;
}

QListWidget::item:hover:!selected {
    background-color: #1a1d26;
}

/* ============================================================
   SPLITTER
   ============================================================ */
QSplitter::handle {
    background-color: #1e2130;
    width: 1px;
}

/* ============================================================
   SCROLLBARS
   ============================================================ */
QScrollBar:vertical {
    background-color: transparent;
    width: 6px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background-color: #353a4c;
    min-height: 30px;
    border-radius: 3px;
}

QScrollBar::handle:vertical:hover {
    background-color: #4a4e60;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background-color: transparent;
    height: 6px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background-color: #353a4c;
    min-width: 30px;
    border-radius: 3px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #4a4e60;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}

/* ============================================================
   LABELS
   ============================================================ */
QLabel#errorLabel {
    color: #f87171;
    font-size: 12px;
}

QLabel#statusLabel {
    color: #606478;
    font-size: 11px;
}

/* ============================================================
   MESSAGE BOX
   ============================================================ */
QMessageBox {
    background-color: #13151c;
}

QMessageBox QLabel {
    color: #e8eaf0;
}

QMessageBox QPushButton {
    min-width: 80px;
}

/* ============================================================
   DIALOG
   ============================================================ */
QDialog {
    background-color: #0d0f14;
}

/* ============================================================
   GROUP BOX
   ============================================================ */
QGroupBox {
    border: 1px solid #282c3a;
    border-radius: 6px;
    margin-top: 14px;
    padding: 18px 14px 10px 14px;
    font-size: 13px;
    font-weight: bold;
    color: #4ade80;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 14px;
    padding: 0 6px;
}

/* ============================================================
   SPIN BOX
   ============================================================ */
QSpinBox {
    background-color: #1a1d26;
    color: #e8eaf0;
    border: 1px solid #282c3a;
    border-radius: 4px;
    padding: 6px 8px;
    min-width: 80px;
}

QSpinBox:focus {
    border-color: #4ade80;
}

QSpinBox::up-button, QSpinBox::down-button {
    background-color: #22262f;
    border: none;
    width: 16px;
    border-radius: 2px;
}

QSpinBox::up-button:hover, QSpinBox::down-button:hover {
    background-color: #2c3040;
}

/* ============================================================
   CHECK BOX
   ============================================================ */
QCheckBox {
    color: #e8eaf0;
    spacing: 8px;
}

QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid #282c3a;
    border-radius: 3px;
    background-color: #1a1d26;
}

QCheckBox::indicator:checked {
    background-color: #22c55e;
    border-color: #22c55e;
}

QCheckBox::indicator:hover {
    border-color: #4ade80;
}

/* ============================================================
   CHANGE PASSWORD BUTTON
   ============================================================ */
QPushButton#changePasswordButton {
    background-color: #0f1a2e;
    border: 1px solid #2563eb;
    border-radius: 4px;
    color: #60a5fa;
    padding: 8px 16px;
}

QPushButton#changePasswordButton:hover {
    background-color: #172554;
    color: #93bbfd;
}

QPushButton#changePasswordButton:disabled {
    background-color: #13151c;
    border-color: #1e2130;
    color: #606478;
}

/* ============================================================
   CANCEL BUTTON
   ============================================================ */
QPushButton#cancelButton {
    background-color: transparent;
    border: 1px solid #282c3a;
    border-radius: 4px;
    color: #a0a4b8;
}

QPushButton#cancelButton:hover {
    border-color: #353a4c;
    color: #e8eaf0;
}

/* ============================================================
   ACTIVITY BAR
   ============================================================ */
QWidget#activityBar {
    background-color: #0d0f14;
    border-right: 1px solid #1e2130;
}

QPushButton#activityButton {
    background-color: transparent;
    color: #606478;
    border: none;
    border-left: 3px solid transparent;
    border-radius: 0;
    font-size: 18px;
    padding: 0;
}

QPushButton#activityButton:hover {
    color: #a0a4b8;
    background-color: #13151c;
}

QPushButton#activityButtonActive {
    background-color: #13151c;
    color: #e8eaf0;
    border: none;
    border-left: 3px solid #4ade80;
    border-radius: 0;
    font-size: 18px;
    padding: 0;
}

/* ============================================================
   SIDEBAR
   ============================================================ */
QWidget#sidebar {
    background-color: #13151c;
    border-right: 1px solid #1e2130;
}

/* Filter Input */
QLineEdit#filterInput {
    background-color: #1a1d26;
    border: 1px solid #282c3a;
    border-radius: 4px;
    padding: 6px 10px;
    margin: 6px;
    font-size: 12px;
    color: #a0a4b8;
}

QLineEdit#filterInput:focus {
    border-color: #4ade80;
    color: #e8eaf0;
}

/* ============================================================
   TAB BAR
   ============================================================ */
QWidget#tabBar {
    background-color: #13151c;
    border-bottom: 1px solid #1e2130;
}

QScrollArea#tabScrollArea {
    background-color: #13151c;
    border: none;
}

QWidget#tabContainer {
    background-color: transparent;
}

QPushButton#tab {
    background-color: #13151c;
    color: #606478;
    border: none;
    border-bottom: 2px solid transparent;
    border-radius: 0;
    padding: 8px 14px;
    font-size: 12px;
    text-align: left;
}

QPushButton#tab:hover {
    color: #a0a4b8;
    background-color: #1a1d26;
}

QPushButton#tabActive {
    background-color: #0d0f14;
    color: #e8eaf0;
    border: none;
    border-bottom: 2px solid #4ade80;
    border-radius: 0;
    padding: 8px 14px;
    font-size: 12px;
    text-align: left;
}

QPushButton#tabCloseButton {
    background-color: transparent;
    color: #606478;
    border: none;
    border-radius: 8px;
    padding: 0;
    font-size: 12px;
    min-width: 18px;
    max-width: 18px;
    min-height: 18px;
    max-height: 18px;
}

QPushButton#tabCloseButton:hover {
    color: #e8eaf0;
    background-color: #2c3040;
}

/* ============================================================
   STATUS BAR
   ============================================================ */
QWidget#statusBar {
    background-color: #13151c;
    border-top: 1px solid #1e2130;
}

QWidget#encryptionSegment {
    background-color: #166534;
}

QLabel#encryptionIndicator {
    color: #4ade80;
    font-size: 11px;
    padding-left: 6px;
    font-weight: 600;
}

QWidget#statusBar QLabel[class="statusItem"] {
    color: #606478;
    font-size: 11px;
}

/* ============================================================
   FORMATTING TOOLBAR
   ============================================================ */
QWidget#formattingToolbar {
    background-color: #13151c;
    border-bottom: 1px solid #1e2130;
}

QPushButton#formatButton {
    background-color: transparent;
    color: #606478;
    border: none;
    border-bottom: 2px solid transparent;
    border-radius: 0;
    padding: 4px 6px;
    font-size: 12px;
    font-weight: bold;
    min-width: 32px;
    min-height: 28px;
    font-family: "Segoe UI", sans-serif;
}

QPushButton#formatButton:hover {
    color: #a0a4b8;
    background-color: #1a1d26;
}

QPushButton#formatButtonActive {
    background-color: #1a1d26;
    color: #4ade80;
    border: none;
    border-bottom: 2px solid #4ade80;
    border-radius: 0;
    padding: 4px 6px;
    font-size: 12px;
    font-weight: bold;
    min-width: 32px;
    min-height: 28px;
    font-family: "Segoe UI", sans-serif;
}

QPushButton#formatButtonActive:hover {
    background-color: #22262f;
    color: #86efac;
}

/* ============================================================
   SEARCH PANEL
   ============================================================ */
QWidget#searchPanel {
    background-color: #13151c;
}

QLineEdit#searchInput {
    background-color: #1a1d26;
    border: 1px solid #282c3a;
    border-radius: 4px;
    padding: 6px 10px;
    margin: 6px;
    font-size: 12px;
    color: #a0a4b8;
}

QLineEdit#searchInput:focus {
    border-color: #4ade80;
    color: #e8eaf0;
}

QLabel#searchResultsCount {
    color: #606478;
    font-size: 11px;
    padding: 2px 10px;
}

QListWidget#searchResultList {
    background-color: #13151c;
    border: none;
    outline: none;
}

/* ============================================================
   TAGS WIDGET
   ============================================================ */
QWidget#tagsWidget {
    background-color: #0d0f14;
    border-bottom: 1px solid #1e2130;
}

QPushButton#tagChip {
    background-color: #0f2318;
    color: #4ade80;
    border: 1px solid #1a3a28;
    border-radius: 12px;
    padding: 2px 10px;
    font-size: 11px;
    font-weight: normal;
}

QPushButton#tagChip:hover {
    background-color: #163a26;
    border-color: #22c55e;
    color: #86efac;
}

QLineEdit#tagInput {
    background-color: transparent;
    color: #606478;
    border: 1px solid #282c3a;
    border-radius: 12px;
    padding: 2px 8px;
    font-size: 11px;
}

QLineEdit#tagInput:focus {
    border-color: #4ade80;
    color: #e8eaf0;
}

/* ============================================================
   FIND BAR
   ============================================================ */
QWidget#findBar {
    background-color: #13151c;
    border-bottom: 1px solid #1e2130;
}

QLineEdit#findInput, QLineEdit#replaceInput {
    background-color: #0d0f14;
    color: #e8eaf0;
    border: 1px solid #282c3a;
    border-radius: 4px;
    padding: 4px 8px;
    font-size: 12px;
}

QLineEdit#findInput:focus, QLineEdit#replaceInput:focus {
    border-color: #4ade80;
}

QLabel#matchLabel {
    color: #606478;
    font-size: 11px;
    font-family: "Cascadia Code", "Consolas", monospace;
}

QPushButton#findButton {
    background-color: transparent;
    color: #606478;
    border: none;
    border-radius: 12px;
    padding: 2px 6px;
    font-size: 12px;
}

QPushButton#findButton:hover {
    color: #e8eaf0;
    background-color: #22262f;
}

)QSS";

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_STYLESHEET_H
