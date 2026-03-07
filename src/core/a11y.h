#ifndef CLEARUI_A11Y_H
#define CLEARUI_A11Y_H

/**
 * Accessibility tree: role, label, state, bounds from retained/declared tree.
 * Platform layer (UI Automation / NSAccessibility / AT-SPI2) can consume this.
 */
struct cui_ctx;
struct cui_node;

typedef enum cui_a11y_role {
	CUI_A11Y_ROLE_NONE,
	CUI_A11Y_ROLE_BUTTON,
	CUI_A11Y_ROLE_CHECKBOX,
	CUI_A11Y_ROLE_TEXTBOX,
	CUI_A11Y_ROLE_LABEL,
	CUI_A11Y_ROLE_GROUP,
	CUI_A11Y_ROLE_COUNT
} cui_a11y_role;

typedef struct cui_a11y_entry {
	cui_a11y_role role;
	const char   *label;   /* from aria_label, label_text, or button_id */
	char          state[32]; /* composable: "checked focused", "checked", "focused", "" */
	float         x, y, w, h;
} cui_a11y_entry;

#define CUI_A11Y_MAX 128

typedef struct cui_a11y_tree {
	cui_a11y_entry entry[CUI_A11Y_MAX];
	int            count;
} cui_a11y_tree;

void cui_a11y_build(struct cui_ctx *ctx, struct cui_node *root, cui_a11y_tree *out);

#endif
