/**
 * ClearUI Public API
 *
 * Declarative immediate-mode GUI: call cui_create once, then each frame call
 * cui_begin_frame, emit your widget tree, cui_end_frame. The library maintains
 * an internal retained tree for layout, diffing, accessibility, and rendering.
 *
 * All coordinates and sizes are in logical pixels. Set cui_config.scale_factor
 * from the platform display scale (e.g. 2.0 on Hi-DPI) so the render path can
 * scale to physical pixels automatically.
 *
 * Color format: 32-bit packed 0xAARRGGBB (alpha in high byte, then red, green,
 * blue in low byte). All color parameters (e.g. cui_style.text_color,
 * cui_draw_rect) use this format. Opaque black is 0xff000000; opaque white
 * 0xffffffff.
 *
 * Thread safety: none. All ClearUI API calls MUST be made from a single
 * thread. No locking is performed; use from multiple threads is undefined
 * behavior.
 *
 * Fixed limits (silent truncation when exceeded):
 *   CUI_PARENT_STACK_MAX   (16)  — max container nesting depth
 *   CUI_FOCUSABLE_MAX      (64)  — max focusable widgets per frame
 *   CUI_A11Y_MAX           (128) — max accessibility entries
 *   CUI_LAST_CLICKED_ID_MAX(64)  — max widget ID length in bytes (incl. NUL)
 *   CUI_FRAME_PRINTF_MAX   (65536) — max cui_frame_printf output in bytes
 */
#ifndef CLEARUI_H
#define CLEARUI_H

#define CUI_VERSION_MAJOR 0
#define CUI_VERSION_MINOR 2
#define CUI_VERSION_PATCH 0

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cui_ctx cui_ctx;
typedef struct cui_platform cui_platform;
typedef struct cui_platform_ctx cui_platform_ctx;
typedef struct cui_rdi cui_rdi;
typedef struct cui_rdi_context cui_rdi_context;

typedef struct cui_config {
	const char *title;
	int        width;           /* logical window width  (pixels) */
	int        height;          /* logical window height (pixels) */
	float      scale_factor;    /* display scale (1.0 = 100%); set from platform for Hi-DPI */
	size_t     draw_buf_capacity; /* 0 = default 1024; max draw commands per buffer */
} cui_config;

/**
 * Layout options for container widgets (row/column/center/stack/wrap).
 * Zero-init is valid: (cui_layout){} gives default gap, no padding, no constraints.
 * If padding > 0, it sets both padding_x and padding_y (explicit values take priority).
 */
typedef struct cui_layout {
	float gap;                  /* space between children (default: 8px) */
	float padding;              /* shorthand for padding_x + padding_y */
	float padding_x, padding_y; /* per-axis inner padding; overrides `padding` if > 0 */
	float max_width, min_width, max_height, min_height; /* size constraints; 0 = none */
	float flex;                 /* flex grow factor (reserved for future flex-grow) */
	int   align;                /* main-axis: CUI_ALIGN_START/CENTER/END/STRETCH */
	int   align_y;              /* cross-axis alignment (row only) */
} cui_layout;

#define CUI_ALIGN_START   0
#define CUI_ALIGN_CENTER  1
#define CUI_ALIGN_END     2
#define CUI_ALIGN_STRETCH 3

/**
 * Style for text and optional widget decoration.
 * Apply via cui_push_style/cui_pop_style (scoped) or cui_label_styled (one-shot).
 */
typedef struct cui_style {
	unsigned int text_color;      /* 0xAARRGGBB; 0 = default (black) */
	int          text_decoration; /* CUI_TEXT_DECORATION_NONE or _STRIKETHROUGH */
} cui_style;

#define CUI_TEXT_DECORATION_NONE 0
#define CUI_TEXT_DECORATION_STRIKETHROUGH 1

cui_ctx *cui_create(const cui_config *config);
void     cui_destroy(cui_ctx *ctx);

void cui_set_platform(cui_ctx *ctx, const cui_platform *platform, cui_platform_ctx *platform_ctx);
void cui_set_rdi(cui_ctx *ctx, const cui_rdi *rdi, cui_rdi_context *rdi_ctx);

int  cui_running(cui_ctx *ctx);
void cui_begin_frame(cui_ctx *ctx);
void cui_end_frame(cui_ctx *ctx);

/**
 * Persistent state keyed by string. Returns a zero-initialized block of `size`
 * bytes on first call; same pointer on subsequent calls with the same key.
 * Pointer is valid for the lifetime of the context.
 */
void *cui_state(cui_ctx *ctx, const char *key, size_t size);

/** Per-frame bump allocation. Returned pointer is valid until the next cui_begin_frame. */
void *cui_frame_alloc(cui_ctx *ctx, size_t size);

/**
 * sprintf into frame-allocated memory. Returned string is valid until the next
 * cui_begin_frame. fmt must be application-controlled (not user input) to avoid
 * format-string risk and unbounded allocation. Max output: 64 KiB.
 */
const char *cui_frame_printf(cui_ctx *ctx, const char *fmt, ...);

void cui_push_style(cui_ctx *ctx, const cui_style *style);
void cui_pop_style(cui_ctx *ctx);

int  cui_button(cui_ctx *ctx, const char *id);
int  cui_checkbox(cui_ctx *ctx, const char *id, int *checked);
void cui_label(cui_ctx *ctx, const char *text);
void cui_label_styled(cui_ctx *ctx, const char *text, const cui_style *style);
void cui_center(cui_ctx *ctx);
void cui_row(cui_ctx *ctx, const cui_layout *opts);
void cui_column(cui_ctx *ctx, const cui_layout *opts);
void cui_stack(cui_ctx *ctx, const cui_layout *opts);
void cui_wrap(cui_ctx *ctx, const cui_layout *opts);
typedef struct cui_scroll_opts { float max_height; } cui_scroll_opts;
typedef struct cui_text_input_opts { const char *placeholder; } cui_text_input_opts;

void cui_scroll(cui_ctx *ctx, const cui_scroll_opts *opts);
void cui_end(cui_ctx *ctx);

void cui_spacer(cui_ctx *ctx, float w, float h);
int  cui_icon_button(cui_ctx *ctx, const char *id, int icon); /* icon: CUI_ICON_* */

int  cui_text_input(cui_ctx *ctx, const char *id, char *buffer, size_t capacity, const cui_text_input_opts *opts);

/** Canvas: custom drawing surface. Coordinates are local to the canvas. Close with cui_end. */
typedef struct cui_canvas_opts { float width; float height; } cui_canvas_opts;
void cui_canvas(cui_ctx *ctx, const cui_canvas_opts *opts);
void cui_draw_rect(cui_ctx *ctx, float x, float y, float w, float h, unsigned int color);   /* color: 0xAARRGGBB */
void cui_draw_circle(cui_ctx *ctx, float cx, float cy, float r, unsigned int color);
void cui_draw_text(cui_ctx *ctx, float x, float y, const char *text, unsigned int color);
#define CUI_ICON_TRASH 1

void cui_inject_click(cui_ctx *ctx, int x, int y);

/* Accessibility: override for next widget */
void cui_aria_label(cui_ctx *ctx, const char *label);
void cui_tab_index(cui_ctx *ctx, int index);

/* Keyboard: inject key for focus/activate (platform or tests). */
#define CUI_KEY_TAB       0x0100
#define CUI_KEY_SHIFT_TAB 0x0101
#define CUI_KEY_ENTER     0x0102
#define CUI_KEY_SPACE     0x0103
#define CUI_KEY_BACKSPACE 0x0104
#define CUI_KEY_DELETE    0x0105
void cui_inject_key(cui_ctx *ctx, int key);

/** Injects one printable character into the focused text input (if any). Codepoint 32–126 (ASCII printable) is inserted at cursor; non-printable is ignored. Processed in end_frame. */
void cui_inject_char(cui_ctx *ctx, unsigned int codepoint);

#ifdef CUI_DEBUG
void cui_dev_overlay(cui_ctx *ctx);  /* frame time, node count, arena usage */
#endif

typedef void (*cui_ui_func)(cui_ctx *ctx);
void cui_run(cui_ctx *ctx, cui_ui_func ui_func);

#ifdef __cplusplus
}
#endif

#endif
