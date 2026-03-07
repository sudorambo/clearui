#include "core/context.h"
#include "arena.h"
#include "frame_alloc.h"
#include "vault.h"
#include "draw_cmd.h"
#include "diff.h"
#include "node.h"
#include "layout/layout.h"
#include "core/render.h"
#include "core/a11y.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define CUI_ARENA_DEFAULT (4 * 1024 * 1024)
#define CUI_PARENT_STACK_MAX 16
#define CUI_LAST_CLICKED_ID_MAX 64
#define CUI_STYLE_STACK_MAX 16
#define CUI_FOCUSABLE_MAX 64

struct cui_ctx {
	cui_arena             arena;
	cui_frame_allocator frame;
	cui_vault       *vault;
	cui_config       config;
	const cui_platform *platform;
	const cui_rdi    *rdi;
	void            *platform_ctx;
	void            *rdi_ctx;
	cui_draw_command_buffer draw_buf;
	cui_draw_command_buffer canvas_cmd_buf;
	cui_draw_command_buffer scaled_buf;
	cui_node        *canvas_node_for_buf;
	cui_node        *declared_root;
	cui_node        *retained_root;
	int             running;
	cui_node        *parent_stack[CUI_PARENT_STACK_MAX];
	int             parent_top;
	cui_style       style_stack[CUI_STYLE_STACK_MAX];
	int             style_top;
	cui_style       current_style;
	int             last_click_x, last_click_y;
	char            last_clicked_id[CUI_LAST_CLICKED_ID_MAX];
	/* Focus / keyboard */
	char            focusable_ids[CUI_FOCUSABLE_MAX][CUI_LAST_CLICKED_ID_MAX];
	int             focusable_count;
	int             focused_index;
	int             next_tab_index;
	const char     *next_aria_label;
	int             pending_key;
	cui_a11y_tree   a11y_tree;
};

typedef struct { char id[CUI_LAST_CLICKED_ID_MAX]; int tab_index; } focusable_t;
static int focusable_cmp(const void *a, const void *b) {
	int ta = ((const focusable_t *)a)->tab_index, tb = ((const focusable_t *)b)->tab_index;
	if (ta < 0) ta = 9999;
	if (tb < 0) tb = 9999;
	return (ta > tb) - (ta < tb);
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

static void process_pending_key(cui_ctx *ctx) {
	int k = ctx->pending_key;
	ctx->pending_key = 0;
	if (ctx->focusable_count <= 0) return;
	switch (k) {
	case 0x0100: /* CUI_KEY_TAB */
		ctx->focused_index = (ctx->focused_index + 1) % ctx->focusable_count;
		break;
	case 0x0101: /* CUI_KEY_SHIFT_TAB */
		ctx->focused_index = (ctx->focused_index - 1 + ctx->focusable_count) % ctx->focusable_count;
		break;
	case 0x0102: /* CUI_KEY_ENTER */
	case 0x0103: /* CUI_KEY_SPACE */
		memcpy(ctx->last_clicked_id, ctx->focusable_ids[ctx->focused_index], CUI_LAST_CLICKED_ID_MAX);
		break;
	default:
		break;
	}
}

cui_ctx *cui_create(const cui_config *config) {
	cui_ctx *ctx = (cui_ctx *)malloc(sizeof(cui_ctx));
	if (!ctx) return NULL;
	memset(ctx, 0, sizeof(cui_ctx));
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

static void hit_test_visit(cui_ctx *ctx, cui_node *n) {
	if (!ctx || !n) return;
	if ((n->type == CUI_NODE_BUTTON || n->type == CUI_NODE_CHECKBOX || n->type == CUI_NODE_ICON_BUTTON) && n->button_id &&
	    (float)ctx->last_click_x >= n->layout_x && (float)ctx->last_click_x < n->layout_x + n->layout_w &&
	    (float)ctx->last_click_y >= n->layout_y && (float)ctx->last_click_y < n->layout_y + n->layout_h) {
		size_t len = strlen(n->button_id);
		if (len >= CUI_LAST_CLICKED_ID_MAX) len = CUI_LAST_CLICKED_ID_MAX - 1;
		memcpy(ctx->last_clicked_id, n->button_id, len + 1);
	}
	for (cui_node *c = n->first_child; c; c = c->next_sibling)
		hit_test_visit(ctx, c);
}

void cui_end_frame(cui_ctx *ctx) {
	if (!ctx) return;
	process_pending_key(ctx);
	cui_diff_run(ctx->declared_root, &ctx->retained_root);
	{
		float vw = (float)(ctx->config.width > 0 ? ctx->config.width : 400);
		float vh = (float)(ctx->config.height > 0 ? ctx->config.height : 300);
		cui_layout_run(ctx->declared_root, vw, vh);
	}
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

const char *cui_frame_printf(cui_ctx *ctx, const char *fmt, ...) {
	if (!ctx || !fmt) return "";
	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);
	if (n < 0) return "";
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
	cui_draw_buf_fini(&ctx->draw_buf);
	cui_draw_buf_fini(&ctx->canvas_cmd_buf);
	cui_draw_buf_fini(&ctx->scaled_buf);
	cui_vault_destroy(ctx->vault);
	cui_frame_allocator_free(&ctx->frame);
	cui_arena_free(&ctx->arena);
	free(ctx);
}

void cui_set_platform(cui_ctx *ctx, const void *platform, void *platform_ctx) {
	if (ctx) { ctx->platform = (const cui_platform *)platform; ctx->platform_ctx = platform_ctx; }
}

void cui_set_rdi(cui_ctx *ctx, const void *rdi, void *rdi_ctx) {
	if (ctx) { ctx->rdi = (const cui_rdi *)rdi; ctx->rdi_ctx = rdi_ctx; }
}

cui_arena *cui_ctx_arena(cui_ctx *ctx) { return ctx ? &ctx->arena : NULL; }
cui_frame_allocator *cui_ctx_frame(cui_ctx *ctx) { return ctx ? &ctx->frame : NULL; }
cui_vault *cui_ctx_vault(cui_ctx *ctx) { return ctx ? ctx->vault : NULL; }
cui_draw_command_buffer *cui_ctx_draw_buf(cui_ctx *ctx) { return ctx ? &ctx->draw_buf : NULL; }
const cui_config *cui_ctx_config(cui_ctx *ctx) { return ctx ? &ctx->config : NULL; }
const void *cui_ctx_rdi(cui_ctx *ctx) { return ctx ? (const void *)ctx->rdi : NULL; }
void *cui_ctx_rdi_ctx(cui_ctx *ctx) { return ctx ? ctx->rdi_ctx : NULL; }
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

void cui_ctx_build_a11y(cui_ctx *ctx) {
	if (ctx) cui_a11y_build(ctx, ctx->declared_root, &ctx->a11y_tree);
}

const cui_a11y_tree *cui_ctx_a11y_tree(cui_ctx *ctx) {
	return ctx ? &ctx->a11y_tree : NULL;
}

int cui_ctx_consume_click(cui_ctx *ctx, const char *id) {
	if (!ctx || !id || !ctx->last_clicked_id[0]) return 0;
	if (strcmp(ctx->last_clicked_id, id) != 0) return 0;
	ctx->last_clicked_id[0] = '\0';
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
