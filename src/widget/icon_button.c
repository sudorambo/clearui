/* Icon button: small button identified by an icon enum. Currently renders icon as a text glyph placeholder. */
#include "../core/context.h"
#include "../core/node.h"
#include "../../include/clearui.h"

int cui_icon_button(cui_ctx *ctx, const char *id, int icon) {
	(void)icon;
	if (!ctx || !id) return 0;
	cui_node *parent = cui_ctx_current_parent(ctx);
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *btn = cui_node_alloc(cui_ctx_arena(ctx));
	if (!btn) return 0;
	btn->type = CUI_NODE_ICON_BUTTON;
	btn->button_id = id;
	btn->icon_id = icon;
	btn->aria_label = cui_ctx_take_aria_label(ctx);
	btn->tab_index = cui_ctx_take_tab_index(ctx);
	if (btn->tab_index < 0) btn->tab_index = -1;
	btn->label_text = (icon == 1) ? "X" : "?"; /* CUI_ICON_TRASH -> X for delete */
	btn->layout_w = 24;
	btn->layout_h = 24;
	if (parent)
		cui_node_append_child(parent, btn);
	else if (root && !*root)
		*root = btn;
	return cui_ctx_consume_click(ctx, id);
}
