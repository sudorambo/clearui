/* Container widgets: row, column, center, stack, wrap, and the shared cui_end closer. */
#include "../core/context.h"
#include "../core/node.h"
#include "../../include/clearui.h"
#include <string.h>

/* Padding resolution per contract: effective_x = padding_x > 0 ? padding_x : padding; same for y. */
static void copy_layout_opts(cui_node_layout *dst, const cui_layout *src) {
	memset(dst, 0, sizeof(*dst));
	if (!src) return;
	dst->gap = src->gap;
	dst->padding_x = src->padding_x > 0 ? src->padding_x : src->padding;
	dst->padding_y = src->padding_y > 0 ? src->padding_y : src->padding;
	dst->max_width = src->max_width;
	dst->min_width = src->min_width;
	dst->max_height = src->max_height;
	dst->min_height = src->min_height;
	dst->flex = src->flex;
	dst->align = src->align;
	dst->align_y = src->align_y;
}

static cui_node *push_container(cui_ctx *ctx, cui_node_type type, const cui_layout *opts, float default_w, float default_h) {
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *n = cui_node_alloc(cui_ctx_arena(ctx));
	if (!n) return NULL;
	n->type = type;
	n->layout_w = default_w;
	n->layout_h = default_h;
	copy_layout_opts(&n->layout_opts, opts);
	if (!*root) *root = n;
	else {
		cui_node *parent = cui_ctx_current_parent(ctx);
		if (parent) cui_node_append_child(parent, n);
		else *root = n;
	}
	cui_ctx_push_parent(ctx, n);
	return n;
}

void cui_center(cui_ctx *ctx) {
	if (!ctx) return;
	push_container(ctx, CUI_NODE_CENTER, NULL, 400, 300);
}

void cui_row(cui_ctx *ctx, const cui_layout *opts) {
	if (!ctx) return;
	push_container(ctx, CUI_NODE_ROW, opts, 0, 0);
}

void cui_column(cui_ctx *ctx, const cui_layout *opts) {
	if (!ctx) return;
	push_container(ctx, CUI_NODE_COLUMN, opts, 0, 0);
}

void cui_stack(cui_ctx *ctx, const cui_layout *opts) {
	if (!ctx) return;
	push_container(ctx, CUI_NODE_STACK, opts, 0, 0);
}

void cui_wrap(cui_ctx *ctx, const cui_layout *opts) {
	if (!ctx) return;
	push_container(ctx, CUI_NODE_WRAP, opts, 0, 0);
}

void cui_end(cui_ctx *ctx) {
	if (ctx) cui_ctx_pop_parent(ctx);
}
