/**
 * cui_ctx: the central state object. Owns all allocators, draw buffers, the
 * declared/retained trees, focus state, style stack, and accessibility tree.
 *
 * Lifetime: cui_create -> (begin_frame/end_frame)* -> cui_destroy.
 * Not thread-safe; all calls must happen on a single thread.
 */
#include "context.h"
#include "../../include/clearui_platform.h"
#include "arena.h"
#include "frame_alloc.h"
#include "vault.h"
#include "draw_cmd.h"
#include "diff.h"
#include "node.h"
#include "theme.h"
#include "../layout/layout.h"
#include "render.h"
#include "a11y.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define CUI_ARENA_DEFAULT (4 * 1024 * 1024)
#define CUI_PARENT_STACK_MAX 16          /* max container nesting depth */
#define CUI_LAST_CLICKED_ID_MAX 64       /* max widget ID length (bytes incl NUL) */
#define CUI_STYLE_STACK_MAX 16           /* max nested push_style calls */
#define CUI_FOCUSABLE_MAX 64             /* max focusable widgets per frame */

struct cui_ctx {
	/* Memory: three allocators with different lifetimes */
	cui_arena             arena;       /* reset each frame; owns declared UI nodes */
	cui_frame_allocator   frame;       /* reset each frame; transient strings (frame_printf) */
	cui_vault            *vault;       /* persistent; application state (cui_state) */

	cui_config       config;
	const cui_platform *platform;
	const cui_rdi    *rdi;
	cui_platform_ctx *platform_ctx;
	cui_rdi_context  *rdi_ctx;

	/* Three draw buffers: main scene, canvas overlay, and Hi-DPI scaled copy */
	cui_draw_command_buffer draw_buf;
	cui_draw_command_buffer canvas_cmd_buf;
	cui_draw_command_buffer scaled_buf;

	cui_node        *canvas_node_for_buf; /* non-NULL while inside a cui_canvas block */
	cui_node        *declared_root;       /* this frame's UI tree (arena-allocated) */
	cui_node        *retained_root;       /* persistent tree for diff/state preservation */
	int              running;

	/* Container nesting stack -- push on cui_row/column/etc, pop on cui_end */
	cui_node        *parent_stack[CUI_PARENT_STACK_MAX];
	int              parent_top;

	cui_style        style_stack[CUI_STYLE_STACK_MAX];
	int              style_top;
	cui_style        current_style;

	/* Input: click coordinates are set by inject_click, consumed during end_frame hit-test */
	int              last_click_x, last_click_y;
	char             last_clicked_id[CUI_LAST_CLICKED_ID_MAX];

	/* Focus / keyboard navigation */
	char             focusable_ids[CUI_FOCUSABLE_MAX][CUI_LAST_CLICKED_ID_MAX];
	int              focusable_count;
	int              focused_index;
	int              next_tab_index;
	const char      *next_aria_label;
	int              pending_key;       /* injected key, consumed at start of end_frame */
	unsigned int     pending_char;      /* injected printable char for focused text input; consumed in end_frame */
	char             text_input_changed_id[CUI_LAST_CLICKED_ID_MAX]; /* id of text input modified last frame; consumed when cui_text_input returns 1 */

	/* Mouse: position and hover (set by inject_mouse_move; hover hit-test in end_frame) */
	int              mouse_x, mouse_y;
	char             hovered_id[CUI_LAST_CLICKED_ID_MAX];
	int              pending_scroll_dx, pending_scroll_dy;

	cui_theme        theme;             /* current theme; draw/layout read via cui_ctx_theme */
	cui_a11y_tree    a11y_tree;
};

/* Focus ordering: widgets with explicit tab_index sort first; -1 means "use declaration order" (sorts last). */
typedef struct { char id[CUI_LAST_CLICKED_ID_MAX]; int tab_index; } focusable_t;
static int focusable_cmp(const void *a, const void *b) {
	int ta = ((const focusable_t *)a)->tab_index, tb = ((const focusable_t *)b)->tab_index;
	if (ta < 0) ta = 9999;
	if (tb < 0) tb = 9999;
	return (ta > tb) - (ta < tb);
}

/* Find a node in the retained tree by button_id (depth-first). */
static cui_node *retained_node_by_id(cui_node *n, const char *id) {
	if (!n || !id) return NULL;
	if (n->button_id && strcmp(n->button_id, id) == 0) return n;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) {
		cui_node *found = retained_node_by_id(c, id);
		if (found) return found;
	}
	return NULL;
}

static void collect_focusable(cui_node *n, focusable_t *list, int *count) {
	if (!n || !list || *count >= CUI_FOCUSABLE_MAX) return;
	const char *id = NULL;
	if (n->type == CUI_NODE_BUTTON || n->type == CUI_NODE_CHECKBOX || n->type == CUI_NODE_ICON_BUTTON || n->type == CUI_NODE_TEXT_INPUT)
		id = n->button_id;
	if (id && id[0]) {
		size_t len = strlen(id);
		if (len >= CUI_LAST_CLICKED_ID_MAX) len = CUI_LAST_CLICKED_ID_MAX - 1;
		memcpy(list[*count].id, id, len + 1);
		list[*count].tab_index = n->tab_index;
		(*count)++;
	}
	for (cui_node *c = n->first_child; c; c = c->next_sibling)
		collect_focusable(c, list, count);
}

static void build_focusable_list(cui_ctx *ctx, cui_node *root) {
	focusable_t list[CUI_FOCUSABLE_MAX];
	int n = 0;
	collect_focusable(root, list, &n);
	qsort(list, (size_t)n, sizeof(list[0]), focusable_cmp);
	ctx->focusable_count = n;
	for (int i = 0; i < n && i < CUI_FOCUSABLE_MAX; i++)
		memcpy(ctx->focusable_ids[i], list[i].id, CUI_LAST_CLICKED_ID_MAX);
	if (ctx->focused_index >= n) ctx->focused_index = n > 0 ? n - 1 : 0;
	if (ctx->focused_index < 0) ctx->focused_index = 0;
}

/**
 * Process a single pending key event. Tab/Shift-Tab cycle focus; Enter/Space
 * synthesize a click on the focused widget (same path as mouse click).
 * Called at the top of end_frame so the key takes effect this frame.
 */
static void process_pending_key(cui_ctx *ctx) {
	int k = ctx->pending_key;
	ctx->pending_key = 0;
	if (ctx->focusable_count <= 0) return;
	switch (k) {
	case 0x0100:
		ctx->focused_index = (ctx->focused_index + 1) % ctx->focusable_count;
		break;
	case 0x0101:
		ctx->focused_index = (ctx->focused_index - 1 + ctx->focusable_count) % ctx->focusable_count;
		break;
	case 0x0102:
	case 0x0103:
		memcpy(ctx->last_clicked_id, ctx->focusable_ids[ctx->focused_index], CUI_LAST_CLICKED_ID_MAX);
		break;
	case 0x0104: /* CUI_KEY_BACKSPACE */
	case 0x0105: /* CUI_KEY_DELETE */ {
		const char *fid = ctx->focusable_count > 0 && ctx->focused_index >= 0 && ctx->focused_index < ctx->focusable_count
			? ctx->focusable_ids[ctx->focused_index] : NULL;
		cui_node *rn = fid && ctx->retained_root ? retained_node_by_id(ctx->retained_root, fid) : NULL;
		if (rn && rn->type == CUI_NODE_TEXT_INPUT && rn->text_input_buf && rn->text_input_cap > 0) {
			size_t len = strlen(rn->text_input_buf);
			int cur = rn->text_input_cursor;
			if (cur < 0) cur = 0;
			if ((size_t)cur > len) cur = 0; /* buffer shortened: clamp to start */
			rn->text_input_cursor = cur;
			if (k == 0x0104 && cur > 0) {
				memmove(rn->text_input_buf + (size_t)(cur - 1), rn->text_input_buf + (size_t)cur, (size_t)(len - (size_t)cur + 1));
				rn->text_input_cursor = cur - 1;
				memcpy(ctx->text_input_changed_id, fid, strlen(fid) + 1);
				if (strlen(ctx->text_input_changed_id) >= CUI_LAST_CLICKED_ID_MAX)
					ctx->text_input_changed_id[CUI_LAST_CLICKED_ID_MAX - 1] = '\0';
			} else if (k == 0x0105 && (size_t)cur < len) {
				memmove(rn->text_input_buf + (size_t)cur, rn->text_input_buf + (size_t)cur + 1, len - (size_t)cur);
				rn->text_input_buf[len - 1] = '\0';
				memcpy(ctx->text_input_changed_id, fid, strlen(fid) + 1);
				if (strlen(ctx->text_input_changed_id) >= CUI_LAST_CLICKED_ID_MAX)
					ctx->text_input_changed_id[CUI_LAST_CLICKED_ID_MAX - 1] = '\0';
			}
		}
		break;
	}
	default:
		break;
	}
}

/* Apply one pending printable character to the focused text input. Called after process_pending_key in end_frame. */
static void apply_pending_char(cui_ctx *ctx) {
	unsigned int cp = ctx->pending_char;
	ctx->pending_char = 0;
	if (cp < 32 || cp > 126) return; /* ASCII printable only */
	if (ctx->focusable_count <= 0 || ctx->focused_index < 0 || ctx->focused_index >= ctx->focusable_count) return;
	const char *fid = ctx->focusable_ids[ctx->focused_index];
	cui_node *rn = ctx->retained_root ? retained_node_by_id(ctx->retained_root, fid) : NULL;
	if (!rn || rn->type != CUI_NODE_TEXT_INPUT || !rn->text_input_buf || rn->text_input_cap == 0) return;
	char *buf = rn->text_input_buf;
	size_t cap = rn->text_input_cap;
	size_t len = strlen(buf);
	int cur = rn->text_input_cursor;
	if (cur < 0) cur = 0;
	if ((size_t)cur > len) cur = (int)len;
	if (len >= cap - 1) return; /* full */
	memmove(buf + (size_t)cur + 1, buf + (size_t)cur, len - (size_t)cur + 1);
	buf[(size_t)cur] = (char)(cp & 0xFF);
	rn->text_input_cursor = cur + 1;
	size_t fid_len = strlen(fid);
	if (fid_len >= CUI_LAST_CLICKED_ID_MAX) fid_len = CUI_LAST_CLICKED_ID_MAX - 1;
	memcpy(ctx->text_input_changed_id, fid, fid_len + 1);
}

cui_ctx *cui_create(const cui_config *config) {
	cui_ctx *ctx = (cui_ctx *)malloc(sizeof(cui_ctx));
	if (!ctx) return NULL;
	memset(ctx, 0, sizeof(cui_ctx));
	ctx->theme.text_color = CUI_THEME_DEFAULT_TEXT_COLOR;
	ctx->theme.button_bg = CUI_THEME_DEFAULT_BUTTON_BG;
	ctx->theme.checkbox_bg = CUI_THEME_DEFAULT_CHECKBOX_BG;
	ctx->theme.input_bg = CUI_THEME_DEFAULT_INPUT_BG;
	ctx->theme.corner_radius = CUI_THEME_DEFAULT_CORNER_RADIUS;
	ctx->theme.font_size = CUI_THEME_DEFAULT_FONT_SIZE;
	ctx->theme.focus_ring_color = CUI_THEME_FOCUS_RING_COLOR;
	ctx->theme.focus_ring_width = CUI_THEME_FOCUS_RING_WIDTH;
	cui_arena_init(&ctx->arena, CUI_ARENA_DEFAULT);
	cui_frame_allocator_init(&ctx->frame, 0);
	ctx->vault = cui_vault_create(0);
	ctx->running = 1;
	if (!ctx->vault) {
		cui_frame_allocator_free(&ctx->frame);
		cui_arena_free(&ctx->arena);
		free(ctx);
		return NULL;
	}
	if (config) {
		ctx->config = *config;
		if (ctx->config.scale_factor <= 0.f) ctx->config.scale_factor = 1.f;
	}
	size_t cap = (config && config->draw_buf_capacity > 0) ? config->draw_buf_capacity : 1024;
	cui_draw_buf_init(&ctx->draw_buf, cap);
	cui_draw_buf_init(&ctx->canvas_cmd_buf, cap);
	cui_draw_buf_init(&ctx->scaled_buf, cap);
	if (ctx->draw_buf.capacity != cap || ctx->canvas_cmd_buf.capacity != cap || ctx->scaled_buf.capacity != cap) {
		cui_draw_buf_fini(&ctx->draw_buf);
		cui_draw_buf_fini(&ctx->canvas_cmd_buf);
		cui_draw_buf_fini(&ctx->scaled_buf);
		cui_vault_destroy(ctx->vault);
		cui_frame_allocator_free(&ctx->frame);
		cui_arena_free(&ctx->arena);
		free(ctx);
		return NULL;
	}
	return ctx;
}

void cui_begin_frame(cui_ctx *ctx) {
	if (!ctx) return;
	cui_arena_reset(&ctx->arena);
	cui_frame_allocator_reset(&ctx->frame);
	ctx->declared_root = NULL;
	ctx->parent_top = 0;
	ctx->style_top = 0;
	memset(&ctx->current_style, 0, sizeof(ctx->current_style));
	ctx->canvas_node_for_buf = NULL;
	cui_draw_buf_clear(&ctx->draw_buf);
	cui_draw_buf_clear(&ctx->canvas_cmd_buf);
	if (ctx->platform && ctx->platform->poll_events && ctx->platform_ctx)
		ctx->running = ctx->platform->poll_events(ctx->platform_ctx) ? 1 : 0;
}

/**
 * Walk the laid-out tree and find which interactive widget (if any) contains
 * the click coordinates. Last match wins (depth-first), so overlapping widgets
 * resolve to the deepest/frontmost.
 */
static void hit_test_visit(cui_ctx *ctx, cui_node *n) {
	if (!ctx || !n) return;
	if ((n->type == CUI_NODE_BUTTON || n->type == CUI_NODE_CHECKBOX || n->type == CUI_NODE_ICON_BUTTON || n->type == CUI_NODE_TEXT_INPUT) && n->button_id &&
	    (float)ctx->last_click_x >= n->layout_x && (float)ctx->last_click_x < n->layout_x + n->layout_w &&
	    (float)ctx->last_click_y >= n->layout_y && (float)ctx->last_click_y < n->layout_y + n->layout_h) {
		size_t len = strlen(n->button_id);
		if (len >= CUI_LAST_CLICKED_ID_MAX) len = CUI_LAST_CLICKED_ID_MAX - 1;
		memcpy(ctx->last_clicked_id, n->button_id, len + 1);
	}
	for (cui_node *c = n->first_child; c; c = c->next_sibling)
		hit_test_visit(ctx, c);
}

/* Hover hit-test: set hovered_id to the topmost interactive node under (mouse_x, mouse_y). Depth-first so last wins. */
static void hover_hit_test_visit(cui_ctx *ctx, cui_node *n) {
	if (!ctx || !n) return;
	if ((n->type == CUI_NODE_BUTTON || n->type == CUI_NODE_CHECKBOX || n->type == CUI_NODE_ICON_BUTTON || n->type == CUI_NODE_TEXT_INPUT) && n->button_id &&
	    (float)ctx->mouse_x >= n->layout_x && (float)ctx->mouse_x < n->layout_x + n->layout_w &&
	    (float)ctx->mouse_y >= n->layout_y && (float)ctx->mouse_y < n->layout_y + n->layout_h) {
		size_t len = strlen(n->button_id);
		if (len >= CUI_LAST_CLICKED_ID_MAX) len = CUI_LAST_CLICKED_ID_MAX - 1;
		memcpy(ctx->hovered_id, n->button_id, len + 1);
		ctx->hovered_id[len] = '\0';
	}
	for (cui_node *c = n->first_child; c; c = c->next_sibling)
		hover_hit_test_visit(ctx, c);
}

/* Find the deepest scroll node under (mouse_x, mouse_y) that has button_id; return it or NULL. */
static cui_node *scroll_node_under_point(cui_ctx *ctx, cui_node *n) {
	if (!ctx || !n) return NULL;
	cui_node *found = NULL;
	if (n->type == CUI_NODE_SCROLL && n->button_id && n->button_id[0] &&
	    (float)ctx->mouse_x >= n->layout_x && (float)ctx->mouse_x < n->layout_x + n->layout_w &&
	    (float)ctx->mouse_y >= n->layout_y && (float)ctx->mouse_y < n->layout_y + n->layout_h)
		found = n;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) {
		cui_node *ch = scroll_node_under_point(ctx, c);
		if (ch) found = ch;
	}
	return found;
}

/* Content height of a scroll node (sum of children layout_h + gaps). */
static float scroll_content_height(cui_node *scroll) {
	if (!scroll) return 0.f;
	float gap = scroll->layout_opts.gap;
	float h = 0.f;
	int n = 0;
	for (cui_node *c = scroll->first_child; c; c = c->next_sibling) {
		h += c->layout_h;
		n++;
	}
	if (n > 1) h += gap * (float)(n - 1);
	return h;
}

/* Apply pending scroll to the scroll node under the pointer; clamp offset. */
static void apply_pending_scroll(cui_ctx *ctx) {
	int dx = ctx->pending_scroll_dx;
	int dy = ctx->pending_scroll_dy;
	ctx->pending_scroll_dx = 0;
	ctx->pending_scroll_dy = 0;
	if (dx == 0 && dy == 0) return;
	cui_node *scroll = scroll_node_under_point(ctx, ctx->declared_root);
	if (!scroll || !scroll->button_id) return;
	cui_node *rn = retained_node_by_id(ctx->retained_root, scroll->button_id);
	if (!rn || rn->type != CUI_NODE_SCROLL) return;
	float content_h = scroll_content_height(scroll);
	float view_h = rn->scroll_max_h > 0 ? rn->scroll_max_h : 150.f;
	rn->scroll_offset_y += (float)dy;
	if (rn->scroll_offset_y < 0.f) rn->scroll_offset_y = 0.f;
	float max_off = content_h - view_h;
	if (max_off < 0.f) max_off = 0.f;
	if (rn->scroll_offset_y > max_off) rn->scroll_offset_y = max_off;
}

/**
 * End-of-frame pipeline:
 *   1. Process keyboard input (Tab/Enter)
 *   2. Diff declared tree against retained tree (state preservation)
 *   3. Layout pass (measure + position all nodes)
 *   4. Build focusable list + accessibility tree
 *   5. Hit-test pending click against laid-out widgets
 *   6. Generate draw command buffer from visual tree
 *   7. Submit to RDI for rendering
 */
void cui_end_frame(cui_ctx *ctx) {
	if (!ctx) return;
	process_pending_key(ctx);
	apply_pending_char(ctx);
	cui_diff_run(ctx->declared_root, &ctx->retained_root);
	{
		float vw = (float)(ctx->config.width > 0 ? ctx->config.width : 400);
		float vh = (float)(ctx->config.height > 0 ? ctx->config.height : 300);
		cui_layout_run(ctx, ctx->declared_root, vw, vh);
	}
	ctx->hovered_id[0] = '\0';
	hover_hit_test_visit(ctx, ctx->declared_root);
	apply_pending_scroll(ctx);
	build_focusable_list(ctx, ctx->declared_root);
	cui_a11y_build(ctx, ctx->declared_root, &ctx->a11y_tree);
	hit_test_visit(ctx, ctx->declared_root);
	cui_build_draw_from_tree(ctx, ctx->declared_root, &ctx->draw_buf, 0.f, 0.f);
#ifdef CUI_DEBUG
	cui_dev_overlay(ctx);
#endif
	cui_render_submit(ctx);
	ctx->last_click_x = -1;
	ctx->last_click_y = -1;
}

void *cui_state(cui_ctx *ctx, const char *key, size_t size) {
	return ctx && key ? cui_vault_get(ctx->vault, key, size) : NULL;
}

void *cui_frame_alloc(cui_ctx *ctx, size_t size) {
	return ctx ? cui_frame_allocator_alloc(&ctx->frame, size) : NULL;
}

/* Hard cap prevents unbounded allocation from attacker-controlled format strings. */
#define CUI_FRAME_PRINTF_MAX 65536

const char *cui_frame_printf(cui_ctx *ctx, const char *fmt, ...) {
	if (!ctx || !fmt) return "";
	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);
	if (n < 0) return "";
	if ((size_t)n >= CUI_FRAME_PRINTF_MAX) return "";
	char *buf = (char *)cui_frame_allocator_alloc(&ctx->frame, (size_t)n + 1);
	if (!buf) return "";
	va_start(ap, fmt);
	vsnprintf(buf, (size_t)n + 1, fmt, ap);
	va_end(ap);
	return buf;
}

void cui_inject_click(cui_ctx *ctx, int x, int y) {
	if (ctx) { ctx->last_click_x = x; ctx->last_click_y = y; }
}

void cui_inject_mouse_move(cui_ctx *ctx, int x, int y) {
	if (ctx) { ctx->mouse_x = x; ctx->mouse_y = y; }
}

void cui_inject_scroll(cui_ctx *ctx, int dx, int dy) {
	if (ctx) { ctx->pending_scroll_dx = dx; ctx->pending_scroll_dy = dy; }
}

const char *cui_ctx_hovered_id(cui_ctx *ctx) {
	if (!ctx || !ctx->hovered_id[0]) return NULL;
	return ctx->hovered_id;
}

int cui_ctx_is_hovered(cui_ctx *ctx, const char *id) {
	if (!ctx || !id) return 0;
	return ctx->hovered_id[0] && strcmp(ctx->hovered_id, id) == 0 ? 1 : 0;
}

void cui_run(cui_ctx *ctx, cui_ui_func ui_func) {
	if (!ctx || !ui_func) return;
	while (cui_running(ctx)) {
		cui_begin_frame(ctx);
		ui_func(ctx);
		cui_end_frame(ctx);
	}
}

int cui_running(cui_ctx *ctx) {
	return ctx ? ctx->running : 0;
}

void cui_destroy(cui_ctx *ctx) {
	if (!ctx) return;
	cui_diff_free_retained(&ctx->retained_root);
	cui_draw_buf_fini(&ctx->draw_buf);
	cui_draw_buf_fini(&ctx->canvas_cmd_buf);
	cui_draw_buf_fini(&ctx->scaled_buf);
	cui_vault_destroy(ctx->vault);
	cui_frame_allocator_free(&ctx->frame);
	cui_arena_free(&ctx->arena);
	free(ctx);
}

static void theme_set_default(cui_theme *t) {
	t->text_color = CUI_THEME_DEFAULT_TEXT_COLOR;
	t->button_bg = CUI_THEME_DEFAULT_BUTTON_BG;
	t->checkbox_bg = CUI_THEME_DEFAULT_CHECKBOX_BG;
	t->input_bg = CUI_THEME_DEFAULT_INPUT_BG;
	t->corner_radius = CUI_THEME_DEFAULT_CORNER_RADIUS;
	t->font_size = CUI_THEME_DEFAULT_FONT_SIZE;
	t->focus_ring_color = CUI_THEME_FOCUS_RING_COLOR;
	t->focus_ring_width = CUI_THEME_FOCUS_RING_WIDTH;
}

void cui_set_theme(cui_ctx *ctx, const cui_theme *theme) {
	if (!ctx) return;
	if (theme)
		ctx->theme = *theme;
	else
		theme_set_default(&ctx->theme);
}

const cui_theme *cui_ctx_theme(cui_ctx *ctx) {
	static cui_theme fallback;
	static int once;
	if (!once) { theme_set_default(&fallback); once = 1; }
	return ctx ? &ctx->theme : &fallback;
}

void cui_theme_dark(cui_theme *out) {
	if (!out) return;
	out->text_color = 0xffe0e0e0u;
	out->button_bg = 0xff404040u;
	out->checkbox_bg = 0xff505050u;
	out->input_bg = 0xff505050u;
	out->corner_radius = CUI_THEME_DEFAULT_CORNER_RADIUS;
	out->font_size = CUI_THEME_DEFAULT_FONT_SIZE;
	out->focus_ring_color = CUI_THEME_FOCUS_RING_COLOR;
	out->focus_ring_width = CUI_THEME_FOCUS_RING_WIDTH;
}

void cui_set_platform(cui_ctx *ctx, const cui_platform *platform, cui_platform_ctx *platform_ctx) {
	if (ctx) { ctx->platform = platform; ctx->platform_ctx = platform_ctx; }
}

void cui_set_rdi(cui_ctx *ctx, const cui_rdi *rdi, cui_rdi_context *rdi_ctx) {
	if (ctx) { ctx->rdi = rdi; ctx->rdi_ctx = rdi_ctx; }
}

cui_arena *cui_ctx_arena(cui_ctx *ctx) { return ctx ? &ctx->arena : NULL; }
cui_frame_allocator *cui_ctx_frame(cui_ctx *ctx) { return ctx ? &ctx->frame : NULL; }
cui_vault *cui_ctx_vault(cui_ctx *ctx) { return ctx ? ctx->vault : NULL; }
cui_draw_command_buffer *cui_ctx_draw_buf(cui_ctx *ctx) { return ctx ? &ctx->draw_buf : NULL; }
const cui_config *cui_ctx_config(cui_ctx *ctx) { return ctx ? &ctx->config : NULL; }
const cui_rdi *cui_ctx_rdi(cui_ctx *ctx) { return ctx ? ctx->rdi : NULL; }
cui_rdi_context *cui_ctx_rdi_ctx(cui_ctx *ctx) { return ctx ? ctx->rdi_ctx : NULL; }
cui_node **cui_ctx_root_ptr(cui_ctx *ctx) { return ctx ? &ctx->declared_root : NULL; }
cui_node *cui_ctx_current_parent(cui_ctx *ctx) {
	return (ctx && ctx->parent_top > 0) ? ctx->parent_stack[ctx->parent_top - 1] : NULL;
}
void cui_ctx_push_parent(cui_ctx *ctx, cui_node *n) {
	if (ctx && ctx->parent_top < CUI_PARENT_STACK_MAX) ctx->parent_stack[ctx->parent_top++] = n;
}
void cui_ctx_pop_parent(cui_ctx *ctx) {
	if (ctx && ctx->parent_top > 0) {
		if (ctx->parent_stack[ctx->parent_top - 1] == ctx->canvas_node_for_buf)
			ctx->canvas_node_for_buf = NULL;
		ctx->parent_top--;
	}
}

void cui_ctx_set_canvas_node(cui_ctx *ctx, cui_node *n) {
	if (ctx) ctx->canvas_node_for_buf = n;
}

cui_node *cui_ctx_canvas_node(cui_ctx *ctx) {
	return ctx ? ctx->canvas_node_for_buf : NULL;
}

cui_draw_command_buffer *cui_ctx_canvas_buf(cui_ctx *ctx) {
	return ctx ? &ctx->canvas_cmd_buf : NULL;
}

cui_draw_command_buffer *cui_ctx_scaled_buf(cui_ctx *ctx) {
	return ctx ? &ctx->scaled_buf : NULL;
}

/* Inside a canvas block, draw commands go to the canvas buffer; otherwise the main scene buffer. */
cui_draw_command_buffer *cui_ctx_current_draw_buf(cui_ctx *ctx) {
	return ctx ? (ctx->canvas_node_for_buf ? &ctx->canvas_cmd_buf : &ctx->draw_buf) : NULL;
}

const char *cui_ctx_focused_id(cui_ctx *ctx) {
	if (!ctx || ctx->focusable_count <= 0 || ctx->focused_index < 0 || ctx->focused_index >= ctx->focusable_count)
		return NULL;
	return ctx->focusable_ids[ctx->focused_index];
}

const char *cui_ctx_take_aria_label(cui_ctx *ctx) {
	if (!ctx) return NULL;
	const char *v = ctx->next_aria_label;
	ctx->next_aria_label = NULL;
	return v;
}

int cui_ctx_take_tab_index(cui_ctx *ctx) {
	if (!ctx) return -1;
	int v = ctx->next_tab_index;
	ctx->next_tab_index = -1;
	return v;
}

void cui_aria_label(cui_ctx *ctx, const char *label) {
	if (ctx) ctx->next_aria_label = label;
}

void cui_tab_index(cui_ctx *ctx, int index) {
	if (ctx) ctx->next_tab_index = index;
}

void cui_inject_key(cui_ctx *ctx, int key) {
	if (ctx) ctx->pending_key = key;
}

void cui_inject_char(cui_ctx *ctx, unsigned int codepoint) {
	if (!ctx) return;
	if (codepoint >= 32 && codepoint <= 126)
		ctx->pending_char = codepoint;
}

#ifdef CUI_DEBUG
static int count_nodes(cui_node *n) {
	if (!n) return 0;
	int c = 1;
	for (cui_node *ch = n->first_child; ch; ch = ch->next_sibling)
		c += count_nodes(ch);
	return c;
}

void cui_dev_overlay(cui_ctx *ctx) {
	if (!ctx) return;
	int n = count_nodes(ctx->declared_root);
	size_t arena_used = ctx->arena.pos;
	const char *msg = cui_frame_printf(ctx, "nodes:%d arena:%zu", n, (unsigned long)arena_used);
	cui_draw_buf_push_text(cui_ctx_draw_buf(ctx), 10.f, 10.f, msg ? msg : "", 0xff0000ff);
}
#endif

cui_node *cui_ctx_declared_root(cui_ctx *ctx) {
	return ctx ? ctx->declared_root : NULL;
}

cui_node *cui_ctx_retained_root(cui_ctx *ctx) {
	return ctx ? ctx->retained_root : NULL;
}

void cui_ctx_build_a11y(cui_ctx *ctx) {
	if (ctx) cui_a11y_build(ctx, ctx->declared_root, &ctx->a11y_tree);
}

const cui_a11y_tree *cui_ctx_a11y_tree(cui_ctx *ctx) {
	return ctx ? &ctx->a11y_tree : NULL;
}

/**
 * Check-and-clear: if the last click targeted widget `id`, clear the ID and
 * return 1. Each click is consumed by exactly one widget per frame.
 */
int cui_ctx_consume_click(cui_ctx *ctx, const char *id) {
	if (!ctx || !id || !ctx->last_clicked_id[0]) return 0;
	if (strcmp(ctx->last_clicked_id, id) != 0) return 0;
	ctx->last_clicked_id[0] = '\0';
	return 1;
}

int cui_ctx_consume_text_input_changed(cui_ctx *ctx, const char *id) {
	if (!ctx || !id || !ctx->text_input_changed_id[0]) return 0;
	if (strcmp(ctx->text_input_changed_id, id) != 0) return 0;
	ctx->text_input_changed_id[0] = '\0';
	return 1;
}

void cui_push_style(cui_ctx *ctx, const cui_style *style) {
	if (!ctx || !style) return;
	if (ctx->style_top < CUI_STYLE_STACK_MAX)
		ctx->style_stack[ctx->style_top++] = ctx->current_style;
	ctx->current_style = *style;
}

void cui_pop_style(cui_ctx *ctx) {
	if (!ctx || ctx->style_top <= 0) return;
	ctx->current_style = ctx->style_stack[--ctx->style_top];
}

const cui_style *cui_ctx_current_style(cui_ctx *ctx) {
	return ctx ? &ctx->current_style : NULL;
}
