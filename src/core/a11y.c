/**
 * Build accessibility tree from declared/retained tree: role, label, state, bounds.
 * Exposed for platform layer (UI Automation, NSAccessibility, AT-SPI2).
 */
#include "a11y.h"
#include "context.h"
#include "node.h"
#include <string.h>

static cui_a11y_role node_to_role(const cui_node *n) {
	switch (n->type) {
	case CUI_NODE_BUTTON:
	case CUI_NODE_ICON_BUTTON: return CUI_A11Y_ROLE_BUTTON;
	case CUI_NODE_CHECKBOX:    return CUI_A11Y_ROLE_CHECKBOX;
	case CUI_NODE_TEXT_INPUT:  return CUI_A11Y_ROLE_TEXTBOX;
	case CUI_NODE_LABEL:       return CUI_A11Y_ROLE_LABEL;
	case CUI_NODE_CENTER:
	case CUI_NODE_ROW:
	case CUI_NODE_COLUMN:
	case CUI_NODE_SCROLL:      return CUI_A11Y_ROLE_GROUP;
	default:                   return CUI_A11Y_ROLE_NONE;
	}
}

static const char *node_label(const cui_node *n) {
	if (n->aria_label && n->aria_label[0]) return n->aria_label;
	if (n->label_text) return n->label_text;
	if (n->button_id) return n->button_id;
	return "";
}

static void visit(cui_node *n, cui_a11y_tree *out, const char *focused_id) {
	if (!n || !out || out->count >= CUI_A11Y_MAX) return;
	cui_a11y_role role = node_to_role(n);
	if (role != CUI_A11Y_ROLE_NONE) {
		cui_a11y_entry *e = &out->entry[out->count++];
		e->role = role;
		e->label = node_label(n);
		e->state = "";
		if (n->type == CUI_NODE_CHECKBOX && n->checkbox_checked && *n->checkbox_checked)
			e->state = "checked";
		if (n->button_id && focused_id && strcmp(n->button_id, focused_id) == 0)
			e->state = "focused";
		e->x = n->layout_x;
		e->y = n->layout_y;
		e->w = n->layout_w;
		e->h = n->layout_h;
	}
	for (cui_node *c = n->first_child; c; c = c->next_sibling)
		visit(c, out, focused_id);
}

void cui_a11y_build(cui_ctx *ctx, cui_node *root, cui_a11y_tree *out) {
	if (!out) return;
	out->count = 0;
	const char *focused_id = cui_ctx_focused_id(ctx);
	visit(root, out, focused_id);
}
