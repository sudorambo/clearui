#ifndef CLEARUI_THEME_H
#define CLEARUI_THEME_H

/**
 * Default theme: colors, corner radii, font size.
 * Applied when no style override is set on a node.
 */
#define CUI_THEME_DEFAULT_TEXT_COLOR      0xff000000u
#define CUI_THEME_DEFAULT_BUTTON_BG      0xffe0e0e0u
#define CUI_THEME_DEFAULT_CHECKBOX_BG    0xffffffffu
#define CUI_THEME_DEFAULT_INPUT_BG       0xffffffffu
#define CUI_THEME_DEFAULT_CORNER_RADIUS  4.f
#define CUI_THEME_DEFAULT_FONT_SIZE      16
/* WCAG 2.1 AA focus indicator (visible outline/ring) */
#define CUI_THEME_FOCUS_RING_COLOR      0xff0066ccu
#define CUI_THEME_FOCUS_RING_WIDTH      2.f

#endif
