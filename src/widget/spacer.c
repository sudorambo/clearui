/* Spacer: invisible sizing element for adding fixed gaps between siblings. */
#include "../core/context.h"
#include "../core/node.h"
#include "../../include/clearui.h"

void cui_spacer(cui_ctx *ctx, float w, float h) {
	if (!ctx) return;
	cui_node *parent = cui_ctx_current_parent(ctx);
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *n = cui_node_alloc(cui_ctx_arena(ctx));
	if (!n) return;
	n->type = CUI_NODE_SPACER;
	n->spacer_w = w > 0 ? w : 8;
	n->spacer_h = h > 0 ? h : 8;
	n->layout_w = n->spacer_w;
	n->layout_h = n->spacer_h;
	if (parent)
		cui_node_append_child(parent, n);
	else if (root && !*root)
		*root = n;
}
