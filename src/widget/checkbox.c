/* Checkbox widget: toggles a caller-owned int on click. Visual state ("[x]"/"[ ]") derived from the boolean. */
#include "../core/context.h"
#include "../core/node.h"
#include "../../include/clearui.h"

int cui_checkbox(cui_ctx *ctx, const char *id, int *checked) {
	if (!ctx || !id) return 0;
	if (!checked) return 0;
	cui_node *parent = cui_ctx_current_parent(ctx);
	cui_node **root = cui_ctx_root_ptr(ctx);
	cui_node *chk = cui_node_alloc(cui_ctx_arena(ctx));
	if (!chk) return 0;
	chk->type = CUI_NODE_CHECKBOX;
	chk->button_id = id;
	chk->checkbox_checked = checked;
	chk->aria_label = cui_ctx_take_aria_label(ctx);
	chk->tab_index = cui_ctx_take_tab_index(ctx);
	if (chk->tab_index < 0) chk->tab_index = -1;
	chk->label_text = *checked ? "[x]" : "[ ]";
	chk->layout_w = 24;
	chk->layout_h = 24;
	if (parent)
		cui_node_append_child(parent, chk);
	else if (root && !*root)
		*root = chk;

	if (cui_ctx_consume_click(ctx, id)) {
		*checked = !*checked;
		return 1;
	}
	return 0;
}
