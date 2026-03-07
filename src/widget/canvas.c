#include "../core/context.h"
#include "../core/node.h"
#include "../core/draw_cmd.h"
#include "../../include/clearui.h"

void cui_canvas(cui_ctx *ctx, const cui_canvas_opts *opts) {
	if (!ctx) return;
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *n = cui_node_alloc(cui_ctx_arena(ctx));
	if (!n) return;
	n->type = CUI_NODE_CANVAS;
	n->layout_w = opts && opts->width > 0 ? opts->width : 200;
	n->layout_h = opts && opts->height > 0 ? opts->height : 200;
	if (!*root) *root = n;
	else {
		cui_node *parent = cui_ctx_current_parent(ctx);
		if (parent) cui_node_append_child(parent, n);
		else *root = n;
	}
	cui_ctx_push_parent(ctx, n);
	cui_ctx_set_canvas_node(ctx, n);
	cui_draw_buf_clear(cui_ctx_canvas_buf(ctx));
}

void cui_draw_rect(cui_ctx *ctx, float x, float y, float w, float h, unsigned int color) {
	if (!ctx) return;
	cui_draw_buf_push_rect(cui_ctx_current_draw_buf(ctx), x, y, w, h, color);
}

void cui_draw_circle(cui_ctx *ctx, float cx, float cy, float r, unsigned int color) {
	if (!ctx) return;
	cui_draw_buf_push_rounded_rect(cui_ctx_current_draw_buf(ctx), cx - r, cy - r, 2*r, 2*r, r, color);
}

void cui_draw_text(cui_ctx *ctx, float x, float y, const char *text, unsigned int color) {
	if (!ctx || !text) return;
	cui_draw_buf_push_text(cui_ctx_current_draw_buf(ctx), x, y, text, color);
}
