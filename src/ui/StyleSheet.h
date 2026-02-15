#ifndef BASTIONX_UI_STYLESHEET_H
#define BASTIONX_UI_STYLESHEET_H

#include <QString>

namespace bastionx {
namespace ui {

// Phase 8: Amber Glassmorphic Cyberpunk UI
// Surface levels: #0f0a08 < #171210 < #1f1a16 < #28221d < #322b25
// Accent amber: #f59e0b / #fbbf24 / #fcd34d / rgba(42, 30, 8, 0.60) (tint)
// Text: #f5f1ed / #b8afa6 / #716b64

// Split into two parts to avoid MSVC's 16KB string literal limit
inline const char* const kStyleSheet_Part1 = R"QSS(

/* ============================================================
   GLOBAL
   ============================================================ */
QWidget {
    background-color: #0f0a08;
    color: #f5f1ed;
    font-family: "Segoe UI", "SF Pro Display", system-ui, sans-serif;
    font-size: 13px;
}

QMainWindow {
    background-color: #0f0a08;
}

/* ============================================================
   TOOLBAR
   ============================================================ */
QToolBar {
    background-color: #0f0a08;
    border-bottom: 1px solid #2a2218;
    padding: 6px 12px;
    spacing: 8px;
}

QToolBar QLabel#titleLabel {
    color: #f59e0b;
    font-size: 15px;
    font-weight: bold;
    letter-spacing: 4px;
}

/* ============================================================
   BUTTONS — Base
   ============================================================ */
QPushButton {
    background-color: #28221d;
    color: #f5f1ed;
    border: 1px solid #342c24;
    border-radius: 4px;
    padding: 8px 16px;
    font-size: 13px;
    font-family: "Segoe UI", sans-serif;
}

QPushButton:hover {
    background-color: #322b25;
    border-color: #3d352d;
}

QPushButton:pressed {
    background-color: #171210;
}

QPushButton:disabled {
    color: #716b64;
    border-color: #2a2218;
    background-color: #171210;
}

/* Submit / Primary Button */
QPushButton#submitButton {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #d97706, stop:1 #b45309);
    border: 1px solid #f59e0b;
    color: #f59e0b;
    font-size: 14px;
    font-weight: bold;
    padding: 12px 28px;
    border-radius: 6px;
    letter-spacing: 2px;
}

QPushButton#submitButton:hover {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #ea580c, stop:1 #d97706);
    color: #fcd34d;
}

QPushButton#submitButton:disabled {
    background-color: #171210;
    border-color: #2a2218;
    color: #716b64;
}

/* Lock Button */
QPushButton#lockButton {
    background-color: transparent;
    border: 1px solid #342c24;
    border-radius: 4px;
    color: #b8afa6;
    padding: 4px 14px;
}

QPushButton#lockButton:hover {
    border-color: #f87171;
    color: #f87171;
    background-color: #2a1218;
}

/* New Note Button */
QPushButton#newNoteButton {
    background-color: #171210;
    border: 1px solid #342c24;
    border-radius: 4px;
    color: #f59e0b;
    padding: 8px 10px;
    text-align: left;
    font-weight: 600;
}

QPushButton#newNoteButton:hover {
    background-color: #1f1a16;
    border-color: #f59e0b;
}

/* Delete Button */
QPushButton#deleteButton {
    background-color: transparent;
    border: 1px solid #342c24;
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
    background-color: #1f1a16;
    color: #f5f1ed;
    border: 1px solid #342c24;
    border-radius: 4px;
    padding: 8px 10px;
    selection-background-color: rgba(42, 30, 8, 0.60);
    font-family: "Segoe UI", sans-serif;
}

QLineEdit:focus {
    border-color: #f59e0b;
}

QLineEdit:disabled {
    color: #716b64;
    background-color: #171210;
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
    border-bottom: 2px solid #2a2218;
    border-radius: 0;
    background-color: #0f0a08;
    padding: 12px 4px;
    color: #f5f1ed;
}

QLineEdit#titleInput:focus {
    border-bottom-color: #f59e0b;
}

/* ============================================================
   RICH TEXT EDITOR
   ============================================================ */
QTextEdit {
    background-color: #0f0a08;
    color: #f5f1ed;
    border: none;
    padding: 12px;
    selection-background-color: rgba(42, 30, 8, 0.60);
    font-family: "Segoe UI", "SF Pro Text", sans-serif;
    font-size: 14px;
}

/* ============================================================
   PLAIN TEXT EDIT (legacy fallback)
   ============================================================ */
QPlainTextEdit {
    background-color: #0f0a08;
    color: #f5f1ed;
    border: none;
    padding: 12px;
    selection-background-color: rgba(42, 30, 8, 0.60);
    font-size: 14px;
}

/* ============================================================
   LIST WIDGET
   ============================================================ */
QListWidget {
    background-color: #171210;
    border: none;
    outline: none;
}

QListWidget::item {
    padding: 10px 10px;
    border-bottom: 1px solid #2a2218;
    border-left: 3px solid transparent;
    color: #f5f1ed;
}

QListWidget::item:selected {
    background-color: rgba(42, 30, 8, 0.60);
    color: #f59e0b;
    border-left: 3px solid #f59e0b;
}

QListWidget::item:hover:!selected {
    background-color: #1f1a16;
}

/* ============================================================
   SPLITTER
   ============================================================ */
QSplitter::handle {
    background-color: #2a2218;
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
    background-color: #3d352d;
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
    background-color: #3d352d;
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
    color: #716b64;
    font-size: 11px;
}

/* ============================================================
   MESSAGE BOX
   ============================================================ */
QMessageBox {
    background-color: #171210;
}

QMessageBox QLabel {
    color: #f5f1ed;
}

QMessageBox QPushButton {
    min-width: 80px;
}

/* ============================================================
   DIALOG
   ============================================================ */
QDialog {
    background-color: #0f0a08;
}

/* ============================================================
   GROUP BOX
   ============================================================ */
QGroupBox {
    border: 1px solid #342c24;
    border-radius: 6px;
    margin-top: 14px;
    padding: 18px 14px 10px 14px;
    font-size: 13px;
    font-weight: bold;
    color: #f59e0b;
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
    background-color: #1f1a16;
    color: #f5f1ed;
    border: 1px solid #342c24;
    border-radius: 4px;
    padding: 6px 8px;
    min-width: 80px;
}

QSpinBox:focus {
    border-color: #f59e0b;
}

QSpinBox::up-button, QSpinBox::down-button {
    background-color: #28221d;
    border: none;
    width: 16px;
    border-radius: 2px;
}

QSpinBox::up-button:hover, QSpinBox::down-button:hover {
    background-color: #322b25;
}

/* ============================================================
   CHECK BOX
   ============================================================ */
QCheckBox {
    color: #f5f1ed;
    spacing: 8px;
}

QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid #342c24;
    border-radius: 3px;
    background-color: #1f1a16;
}

QCheckBox::indicator:checked {
    background-color: #f59e0b;
    border-color: #f59e0b;
}

QCheckBox::indicator:hover {
    border-color: #f59e0b;
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
    background-color: #171210;
    border-color: #2a2218;
    color: #716b64;
}

/* ============================================================
   CANCEL BUTTON
   ============================================================ */
QPushButton#cancelButton {
    background-color: transparent;
    border: 1px solid #342c24;
    border-radius: 4px;
    color: #b8afa6;
}

QPushButton#cancelButton:hover {
    border-color: #3d352d;
    color: #f5f1ed;
}

)QSS";

inline const char* const kStyleSheet_Part2 = R"QSS(

/* ============================================================
   ACTIVITY BAR
   ============================================================ */
QWidget#activityBar {
    background-color: #0f0a08;
    border-right: 1px solid #2a2218;
}

QPushButton#activityButton {
    background-color: transparent;
    color: #716b64;
    border: none;
    border-left: 3px solid transparent;
    border-radius: 0;
    font-size: 18px;
    padding: 0;
}

QPushButton#activityButton:hover {
    color: #b8afa6;
    background-color: #171210;
}

QPushButton#activityButtonActive {
    background-color: #171210;
    color: #f5f1ed;
    border: none;
    border-left: 3px solid #f59e0b;
    border-radius: 0;
    font-size: 18px;
    padding: 0;
}

/* ============================================================
   MODE SELECTOR BAR (Sliding Blade Edge Selector)
   ============================================================ */
QWidget#modeSelectorBar {
    background-color: #171210;
    border-bottom: 1px solid #2a2218;
}

/* Segment labels */
QLabel#modeSegmentInactive {
    color: #716b64;
    font-size: 11px;
    font-weight: bold;
    letter-spacing: 2px;
    padding: 8px;
    background-color: transparent;
}

QLabel#modeSegmentInactive:hover {
    color: #b8afa6;
}

QLabel#modeSegmentActive {
    color: #fcd34d;
    font-size: 11px;
    font-weight: bold;
    letter-spacing: 2px;
    padding: 8px;
    background-color: transparent;
}

/* Blade indicator */
QFrame#bladeIndicator {
    background-color: #f59e0b;
    border: none;
    border-radius: 2px;
}

/* ============================================================
   SIDEBAR
   ============================================================ */
QWidget#sidebar {
    background-color: #171210;
    border-right: 1px solid #2a2218;
}

/* Filter Input */
QLineEdit#filterInput {
    background-color: #1f1a16;
    border: 1px solid #342c24;
    border-radius: 4px;
    padding: 6px 10px;
    margin: 6px;
    font-size: 12px;
    color: #b8afa6;
}

QLineEdit#filterInput:focus {
    border-color: #f59e0b;
    color: #f5f1ed;
}

/* ============================================================
   TAB BAR
   ============================================================ */
QWidget#tabBar {
    background-color: #171210;
    border-bottom: 1px solid #2a2218;
}

QScrollArea#tabScrollArea {
    background-color: #171210;
    border: none;
}

QWidget#tabContainer {
    background-color: transparent;
}

QPushButton#tab {
    background-color: transparent;
    color: #716b64;
    border: none;
    border-radius: 0;
    padding: 8px 12px;
    font-size: 12px;
    font-weight: 600;
    letter-spacing: 1.5px;
    text-align: left;
}

QPushButton#tab:hover {
    color: #b8afa6;
    background-color: #1f1a16;
}

QPushButton#tabActive {
    background-color: transparent;
    color: #fcd34d;
    border: none;
    border-radius: 0;
    padding: 8px 12px;
    font-size: 12px;
    font-weight: 600;
    letter-spacing: 1.5px;
    text-align: left;
}

QPushButton#tabCloseButton {
    background-color: transparent;
    color: #716b64;
    border: none;
    border-radius: 8px;
    padding: 0;
    font-size: 14px;
    min-width: 16px;
    max-width: 16px;
    min-height: 16px;
    max-height: 16px;
}

QPushButton#tabCloseButton:hover {
    color: #f87171;
    background-color: rgba(248, 113, 113, 0.15);
}

/* Tab blade indicator */
QFrame#tabBladeIndicator {
    background-color: #f59e0b;
    border: none;
    border-radius: 2px 2px 0 0;
}

/* Tab separators */
QFrame#tabSeparator {
    background-color: rgba(61, 46, 24, 0.40);
    border: none;
}

/* ============================================================
   STATUS BAR
   ============================================================ */
QWidget#statusBar {
    background-color: #171210;
    border-top: 1px solid #2a2218;
}

QWidget#encryptionSegment {
    background-color: #d97706;
}

QLabel#encryptionIndicator {
    color: #f59e0b;
    font-size: 11px;
    padding-left: 6px;
    font-weight: 600;
}

QWidget#statusBar QLabel[class="statusItem"] {
    color: #716b64;
    font-size: 11px;
}

/* ============================================================
   FORMATTING TOOLBAR
   ============================================================ */
QWidget#formattingToolbar {
    background-color: #171210;
    border-bottom: 1px solid #2a2218;
}

QPushButton#formatButton {
    background-color: transparent;
    color: #716b64;
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
    color: #b8afa6;
    background-color: #1f1a16;
}

QPushButton#formatButtonActive {
    background-color: #1f1a16;
    color: #f59e0b;
    border: none;
    border-bottom: 2px solid #f59e0b;
    border-radius: 0;
    padding: 4px 6px;
    font-size: 12px;
    font-weight: bold;
    min-width: 32px;
    min-height: 28px;
    font-family: "Segoe UI", sans-serif;
}

QPushButton#formatButtonActive:hover {
    background-color: #28221d;
    color: #fcd34d;
}

/* ============================================================
   SEARCH PANEL
   ============================================================ */
QWidget#searchPanel {
    background-color: #171210;
}

QLineEdit#searchInput {
    background-color: #1f1a16;
    border: 1px solid #342c24;
    border-radius: 4px;
    padding: 6px 10px;
    margin: 6px;
    font-size: 12px;
    color: #b8afa6;
}

QLineEdit#searchInput:focus {
    border-color: #f59e0b;
    color: #f5f1ed;
}

QLabel#searchResultsCount {
    color: #716b64;
    font-size: 11px;
    padding: 2px 10px;
}

QListWidget#searchResultList {
    background-color: #171210;
    border: none;
    outline: none;
}

/* ============================================================
   TAGS WIDGET
   ============================================================ */
QWidget#tagsWidget {
    background-color: #0f0a08;
    border-bottom: 1px solid #2a2218;
}

QPushButton#tagChip {
    background-color: rgba(42, 30, 8, 0.60);
    color: #f59e0b;
    border: 1px solid rgba(61, 46, 24, 0.80);
    border-radius: 12px;
    padding: 2px 10px;
    font-size: 11px;
    font-weight: normal;
}

QPushButton#tagChip:hover {
    background-color: rgba(61, 46, 24, 0.70);
    border-color: #f59e0b;
    color: #fcd34d;
}

QLineEdit#tagInput {
    background-color: transparent;
    color: #716b64;
    border: 1px solid #342c24;
    border-radius: 12px;
    padding: 2px 8px;
    font-size: 11px;
}

QLineEdit#tagInput:focus {
    border-color: #f59e0b;
    color: #f5f1ed;
}

/* ============================================================
   FIND BAR
   ============================================================ */
QWidget#findBar {
    background-color: #171210;
    border-bottom: 1px solid #2a2218;
}

QLineEdit#findInput, QLineEdit#replaceInput {
    background-color: #0f0a08;
    color: #f5f1ed;
    border: 1px solid #342c24;
    border-radius: 4px;
    padding: 4px 8px;
    font-size: 12px;
}

QLineEdit#findInput:focus, QLineEdit#replaceInput:focus {
    border-color: #f59e0b;
}

QLabel#matchLabel {
    color: #716b64;
    font-size: 11px;
    font-family: "Cascadia Code", "Consolas", monospace;
}

QPushButton#findButton {
    background-color: transparent;
    color: #716b64;
    border: none;
    border-radius: 12px;
    padding: 2px 6px;
    font-size: 12px;
}

QPushButton#findButton:hover {
    color: #f5f1ed;
    background-color: #28221d;
}

/* ============================================================
   UNLOCK SCREEN
   ============================================================ */
QLabel#unlockTitle {
    color: #f59e0b;
    font-size: 32px;
    font-weight: bold;
    letter-spacing: 3px;
}

QLabel#unlockStatus {
    color: #b8afa6;
    font-size: 13px;
}

/* ============================================================
   FORMATTING TOOLBAR SEPARATOR
   ============================================================ */
QWidget#formatSeparator {
    background-color: #342c24;
}

)QSS";

// Concatenate the two parts into a QString
inline QString getStyleSheet() {
    return QString(kStyleSheet_Part1) + QString(kStyleSheet_Part2);
}

// For backward compatibility, create a QString that's returned by value
inline const QString kStyleSheet = QString(kStyleSheet_Part1) + QString(kStyleSheet_Part2);

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_STYLESHEET_H
