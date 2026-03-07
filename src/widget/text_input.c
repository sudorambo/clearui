/* Text input widget: displays a caller-owned char buffer. Keyboard editing not yet wired (stub returns 0). */
#include "../core/context.h"
#include "../core/node.h"
#include "../../include/clearui.h"
#include <string.h>

int cui_text_input(cui_ctx *ctx, const char *id, char *buffer, size_t capacity, const cui_text_input_opts *opts) {
	if (!ctx || !id || !buffer || capacity == 0) return 0;
	cui_node *parent = cui_ctx_current_parent(ctx);
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *n = cui_node_alloc(cui_ctx_arena(ctx));
	if (!n) return 0;
	n->type = CUI_NODE_TEXT_INPUT;
	n->button_id = id;
	n->aria_label = cui_ctx_take_aria_label(ctx);
	n->tab_index = cui_ctx_take_tab_index(ctx);
	if (n->tab_index < 0) n->tab_index = -1;
	n->text_input_buf = buffer;
	n->text_input_cap = capacity;
	n->text_input_cursor = 0; /* diff copies from retained when present */
	if (opts && opts->placeholder)
		n->label_text = opts->placeholder;
	n->layout_w = 120;
	n->layout_h = 24;
	if (parent)
		cui_node_append_child(parent, n);
	else if (root && !*root)
		*root = n;
	return cui_ctx_consume_text_input_changed(ctx, id);
}
