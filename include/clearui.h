/**
 * ClearUI Public API
 * Zero-to-window: cui_create, cui_begin_frame, cui_center/cui_label, cui_end_frame, cui_destroy.
 *
 * All coordinates and sizes (layout, draw) are in logical pixels. Set cui_config.scale_factor
 * from the platform display scale (e.g. 2.0 on Hi-DPI) so the render path can scale to physical pixels.
 */
#ifndef CLEARUI_H
#define CLEARUI_H

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
	int        width;
	int        height;
	float      scale_factor;   /* display scale (1.0 = 100%); set from platform for Hi-DPI */
	size_t     draw_buf_capacity; /* 0 = default 1024; >0 = max draw commands per buffer */
} cui_config;

/* Layout options for row/column/center/stack/wrap. (cui_layout){} is valid. */
typedef struct cui_layout {
	float gap;
	float padding;
	float padding_x, padding_y;
	float max_width, min_width, max_height, min_height;
	float flex;
	int   align, align_y;
} cui_layout;

#define CUI_ALIGN_START   0
#define CUI_ALIGN_CENTER  1
#define CUI_ALIGN_END     2
#define CUI_ALIGN_STRETCH 3

/* Style for text (and optional widget styling). Applied via push_style/pop_style or label_styled. */
typedef struct cui_style {
	unsigned int text_color;   /* 0 = use default (black) */
	int          text_decoration; /* 0=none, 1=strikethrough */
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

void *cui_state(cui_ctx *ctx, const char *key, size_t size);
void *cui_frame_alloc(cui_ctx *ctx, size_t size);       /* valid until next cui_begin_frame */
const char *cui_frame_printf(cui_ctx *ctx, const char *fmt, ...); /* valid until next cui_begin_frame */

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

/* Canvas: custom drawing; list valid until next cui_begin_frame */
typedef struct cui_canvas_opts { float width; float height; } cui_canvas_opts;
void cui_canvas(cui_ctx *ctx, const cui_canvas_opts *opts);
void cui_draw_rect(cui_ctx *ctx, float x, float y, float w, float h, unsigned int color);
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
void cui_inject_key(cui_ctx *ctx, int key);

#ifdef CUI_DEBUG
void cui_dev_overlay(cui_ctx *ctx);  /* frame time, node count, arena usage */
#endif

typedef void (*cui_ui_func)(cui_ctx *ctx);
void cui_run(cui_ctx *ctx, cui_ui_func ui_func);

#ifdef __cplusplus
}
#endif

#endif
