#include "core/context.h"
#include "core/node.h"
#include "clearui.h"

void cui_scroll(cui_ctx *ctx, const cui_scroll_opts *opts) {
	if (!ctx) return;
	const cui_scroll_opts *o = opts;
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *scroll = cui_node_alloc(cui_ctx_arena(ctx));
	if (!scroll) return;
	scroll->type = CUI_NODE_SCROLL;
	scroll->scroll_max_h = o && o->max_height > 0 ? o->max_height : 150;
	scroll->scroll_offset_y = 0;
	if (!*root) *root = scroll;
	else {
		cui_node *parent = cui_ctx_current_parent(ctx);
		if (parent) cui_node_append_child(parent, scroll);
		else *root = scroll;
	}
	cui_ctx_push_parent(ctx, scroll);
}
