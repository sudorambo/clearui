#ifndef CLEARUI_NODE_H
#define CLEARUI_NODE_H

/**
 * Internal UI tree node. Each widget or container creates one node per frame
 * via cui_node_alloc (arena-allocated). Children are an intrusive singly-linked
 * list (first_child + next_sibling) with a last_child pointer for O(1) append.
 */

#include <stddef.h>

typedef enum cui_node_type {
	CUI_NODE_NONE,
	CUI_NODE_LABEL,
	CUI_NODE_BUTTON,
	CUI_NODE_CHECKBOX,
	CUI_NODE_ICON_BUTTON,
	CUI_NODE_SPACER,
	CUI_NODE_SCROLL,
	CUI_NODE_TEXT_INPUT,
	CUI_NODE_CANVAS,
	CUI_NODE_CENTER,
	CUI_NODE_ROW,
	CUI_NODE_COLUMN,
	CUI_NODE_STACK,
	CUI_NODE_WRAP,
	CUI_NODE_COUNT
} cui_node_type;

/* Internal layout options (mirrors public cui_layout for container nodes). */
typedef struct cui_node_layout {
	float gap;
	float padding_x, padding_y;
	float max_width, min_width, max_height, min_height;
	float flex;
	int   align, align_y;
} cui_node_layout;

typedef struct cui_node cui_node;

struct cui_node {
	cui_node_type type;
	cui_node          *first_child;
	cui_node          *last_child;
	cui_node          *next_sibling;
	const char       *label_text;
	const char       *button_id;
	const char       *aria_label;   /* override for a11y; screen reader uses this or label_text/button_id */
	int               tab_index;    /* -1 = declaration order; else explicit tab order */
	unsigned int      text_color;
	int               text_decoration;
	int              *checkbox_checked;   /* for CHECKBOX */
	char             *text_input_buf;     /* for TEXT_INPUT */
	size_t            text_input_cap;
	int               text_input_cursor;  /* for TEXT_INPUT: index in [0, strlen(buffer)]; insertion point */
	float             scroll_offset_y;   /* for SCROLL */
	float             scroll_max_h;
	float             spacer_w, spacer_h; /* for SPACER */
	int               icon_id;            /* for ICON_BUTTON */
	cui_node_layout   layout_opts;
	float             layout_x, layout_y, layout_w, layout_h;
};

struct cui_arena;

void cui_node_tree_clear(cui_node **root);
cui_node *cui_node_alloc(struct cui_arena *arena);
void cui_node_append_child(cui_node *parent, cui_node *child);

#endif
