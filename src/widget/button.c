#include "../core/context.h"
#include "../core/node.h"
#include "../../include/clearui.h"

int cui_button(cui_ctx *ctx, const char *id) {
	if (!ctx || !id) return 0;
	cui_node *parent = cui_ctx_current_parent(ctx);
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *btn = cui_node_alloc(cui_ctx_arena(ctx));
	if (!btn) return 0;
	btn->type = CUI_NODE_BUTTON;
	btn->button_id = id;
	btn->label_text = id;
	btn->aria_label = cui_ctx_take_aria_label(ctx);
	btn->tab_index = cui_ctx_take_tab_index(ctx);
	if (btn->tab_index < 0) btn->tab_index = -1;
	btn->layout_w = 80;
	btn->layout_h = 24;
	{
		const cui_style *s = cui_ctx_current_style(ctx);
		if (s) { btn->text_color = s->text_color; btn->text_decoration = s->text_decoration; }
	}
	if (parent)
		cui_node_append_child(parent, btn);
	else if (root && !*root)
		*root = btn;

	return cui_ctx_consume_click(ctx, id);
}
