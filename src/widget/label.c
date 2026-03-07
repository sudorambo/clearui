#include "core/context.h"
#include "core/node.h"
#include "clearui.h"
#include <string.h>

void cui_label(cui_ctx *ctx, const char *text) {
	if (!ctx || !text) return;
	cui_node *parent = cui_ctx_current_parent(ctx);
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *label = cui_node_alloc(cui_ctx_arena(ctx));
	if (!label) return;
	label->type = CUI_NODE_LABEL;
	label->label_text = text;
	label->layout_x = 10;
	label->layout_y = 10;
	label->layout_w = 100;
	label->layout_h = 20;
	{
		const cui_style *s = cui_ctx_current_style(ctx);
		if (s) { label->text_color = s->text_color; label->text_decoration = s->text_decoration; }
	}
	if (parent)
		cui_node_append_child(parent, label);
	else if (root && !*root)
		*root = label;
}

void cui_label_styled(cui_ctx *ctx, const char *text, const cui_style *style) {
	if (!ctx || !text) return;
	cui_style zero = {0};
	cui_push_style(ctx, style ? style : &zero);
	cui_label(ctx, text);
	cui_pop_style(ctx);
}
