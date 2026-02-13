#ifndef BASTIONX_UI_UICONSTANTS_H
#define BASTIONX_UI_UICONSTANTS_H

namespace bastionx {
namespace ui {
namespace constants {

// ============================================================
// BASELINE GRID SYSTEM (8px base)
// ============================================================
constexpr int kGridBase = 8;
constexpr int kGrid1x = 8;   // 8px
constexpr int kGrid2x = 16;  // 16px
constexpr int kGrid3x = 24;  // 24px
constexpr int kGrid4x = 32;  // 32px
constexpr int kGrid5x = 40;  // 40px
constexpr int kGrid6x = 48;  // 48px

// ============================================================
// SPACING CONSTANTS
// ============================================================
// Margins
constexpr int kMarginTiny = 4;
constexpr int kMarginSmall = kGrid1x;    // 8px
constexpr int kMarginMedium = kGrid2x;   // 16px
constexpr int kMarginLarge = kGrid3x;    // 24px

// Element spacing
constexpr int kSpacingTight = 4;
constexpr int kSpacingNormal = kGrid1x;   // 8px
constexpr int kSpacingRelaxed = kGrid2x;  // 16px

// Section spacing (creates visual hierarchy)
constexpr int kSpacingSection = kGrid3x;  // 24px between major sections

// ============================================================
// COMPONENT HEIGHTS (aligned to 8px grid)
// ============================================================
constexpr int kActivityBarWidth = kGrid6x;      // 48px
constexpr int kTabBarHeight = kGrid4x;          // 32px (was 35px - not grid-aligned)
constexpr int kFormattingToolbarHeight = kGrid4x; // 32px
constexpr int kStatusBarHeight = kGrid3x;       // 24px
constexpr int kTagsWidgetHeight = kGrid4x;      // 32px (was 28px)
constexpr int kButtonHeightStandard = kGrid4x;  // 32px
constexpr int kButtonHeightCompact = kGrid3x;   // 24px
constexpr int kInputHeightStandard = kGrid4x;   // 32px

// ============================================================
// COMPONENT WIDTHS
// ============================================================
constexpr int kSidebarMinWidth = 240;
constexpr int kSidebarMaxWidth = 400;
constexpr int kSidebarDefaultWidth = 280;

// Tab sizing
constexpr int kTabMinWidth = 80;
constexpr int kTabMaxWidth = 200;

// Tag input
constexpr int kTagInputWidth = 100;  // was 80px - too narrow

// ============================================================
// BORDER RADIUS
// ============================================================
constexpr int kRadiusSmall = 4;
constexpr int kRadiusMedium = 6;
constexpr int kRadiusLarge = 8;

// ============================================================
// BUTTON SIZING
// ============================================================
constexpr int kButtonMinWidth = 80;
constexpr int kIconButtonSize = kGrid4x;       // 32x32 for icon buttons
constexpr int kCloseButtonSize = 20;           // 20x20 for tab close
constexpr int kActivityButtonSize = kGrid6x;   // 48x48

// ============================================================
// ANIMATION TIMING (milliseconds)
// ============================================================
constexpr int kFadeInDuration = 150;
constexpr int kFadeOutDuration = 100;
constexpr int kHoverTransition = 120;
constexpr int kPressTransition = 80;

// ============================================================
// RESPONSIVE BREAKPOINTS
// ============================================================
constexpr int kSplitterMinEditorWidth = 500;
constexpr double kSplitterSidebarRatio = 0.30;  // 30% sidebar, 70% editor

}  // namespace constants
}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_UICONSTANTS_H
