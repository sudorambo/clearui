/**
 * Diff engine: reconcile this frame's declared tree against the retained tree
 * to preserve persistent widget state (scroll offset, checkbox, text buffer)
 * across frames. Matching uses button_id (key) first, then positional index.
 * Unmatched retained nodes are freed; new declared nodes get fresh retained clones.
 */
#include "diff.h"
#include "node.h"
#include <stdlib.h>
#include <string.h>

static void free_retained_tree(cui_node *n) {
	if (!n) return;
	for (cui_node *c = n->first_child; c; ) {
		cui_node *next = c->next_sibling;
		free_retained_tree(c);
		c = next;
	}
	free(n);
}

void cui_diff_free_retained(cui_node **retained) {
	if (!retained) return;
	if (*retained) {
		free_retained_tree(*retained);
		*retained = NULL;
	}
}

/* Allocate a retained node and copy from declared (structure + initial state). */
static cui_node *clone_retained_from_declared(const cui_node *declared) {
	cui_node *r = (cui_node *)malloc(sizeof(cui_node));
	if (!r) return NULL;
	memset(r, 0, sizeof(cui_node));
	r->type = declared->type;
	r->layout_opts = declared->layout_opts;
	r->label_text = declared->label_text;
	r->button_id = declared->button_id;
	r->aria_label = declared->aria_label;
	r->tab_index = declared->tab_index;
	r->text_color = declared->text_color;
	r->text_decoration = declared->text_decoration;
	r->scroll_offset_y = declared->scroll_offset_y;
	r->scroll_max_h = declared->scroll_max_h;
	r->spacer_w = declared->spacer_w;
	r->spacer_h = declared->spacer_h;
	r->icon_id = declared->icon_id;
	r->checkbox_checked = declared->checkbox_checked;
	r->text_input_buf = declared->text_input_buf;
	r->text_input_cap = declared->text_input_cap;
	r->first_child = r->last_child = r->next_sibling = NULL;
	return r;
}

/* Copy persistent state from retained into declared. */
static void copy_state_retained_to_declared(cui_node *declared, const cui_node *retained) {
	if (!declared || !retained) return;
	switch (retained->type) {
	case CUI_NODE_SCROLL:
		declared->scroll_offset_y = retained->scroll_offset_y;
		break;
	case CUI_NODE_TEXT_INPUT:
		declared->text_input_buf = retained->text_input_buf;
		declared->text_input_cap = retained->text_input_cap;
		break;
	case CUI_NODE_CHECKBOX:
		declared->checkbox_checked = retained->checkbox_checked;
		break;
	default:
		break;
	}
}

/**
 * Two-phase child reconciliation:
 *   1. Match declared children to retained children (by key, then position).
 *   2. Free unmatched retained children; clone unmatched declared children.
 * This is intentionally O(n*m) for simplicity; typical UI trees have few children per node.
 */
static void reconcile(cui_node *declared, cui_node **retained) {
	if (!retained) return;
	if (!declared) {
		cui_diff_free_retained(retained);
		return;
	}
	int num_decl = 0, num_ret = 0;
	for (cui_node *c = declared->first_child; c; c = c->next_sibling) num_decl++;
	for (cui_node *c = *retained ? (*retained)->first_child : NULL; c; c = c->next_sibling) num_ret++;

	cui_node **ret_arr = NULL;
	int *used = NULL;
	if (num_ret > 0) {
		ret_arr = (cui_node **)malloc((size_t)num_ret * sizeof(cui_node *));
		used = (int *)malloc((size_t)num_ret * sizeof(int));
		if (!ret_arr || !used) {
			free(ret_arr);
			free(used);
			ret_arr = NULL;
			used = NULL;
		} else {
			memset(used, 0, (size_t)num_ret * sizeof(int));
			int i = 0;
			for (cui_node *c = *retained ? (*retained)->first_child : NULL; c && i < num_ret; c = c->next_sibling, i++)
				ret_arr[i] = c;
		}
	}

	cui_node *new_first = NULL, *new_last = NULL;
	int decl_i = 0;
	for (cui_node *d = declared->first_child; d; d = d->next_sibling, decl_i++) {
		cui_node *r = NULL;
		if (ret_arr && used) {
			for (int idx = 0; idx < num_ret; idx++) {
				if (used[idx]) continue;
				cui_node *x = ret_arr[idx];
				if (d->button_id && x->button_id && strcmp(d->button_id, x->button_id) == 0) {
					r = x; used[idx] = 1; break;
				}
			}
			if (!r)
				for (int idx = 0; idx < num_ret; idx++) {
					if (used[idx]) continue;
					if (idx == decl_i) { r = ret_arr[idx]; used[idx] = 1; break; }
				}
		}
		if (r) {
			r->type = d->type;
			r->layout_opts = d->layout_opts;
			r->label_text = d->label_text;
			r->button_id = d->button_id;
			r->aria_label = d->aria_label;
			r->tab_index = d->tab_index;
			r->text_color = d->text_color;
			r->text_decoration = d->text_decoration;
			r->scroll_max_h = d->scroll_max_h;
			r->spacer_w = d->spacer_w;
			r->spacer_h = d->spacer_h;
			r->icon_id = d->icon_id;
			r->next_sibling = NULL;
			reconcile(d, &r);
			copy_state_retained_to_declared(d, r);
			if (!new_first) new_first = new_last = r;
			else { new_last->next_sibling = r; new_last = r; }
		} else {
			cui_node *n = clone_retained_from_declared(d);
			if (n) {
				reconcile(d, &n);
				copy_state_retained_to_declared(d, n);
				if (!new_first) new_first = new_last = n;
				else { new_last->next_sibling = n; new_last = n; }
			}
		}
	}
	/* Free retained children that were not matched (use array; list was re-linked). */
	if (ret_arr && used) {
		for (int idx = 0; idx < num_ret; idx++)
			if (!used[idx])
				free_retained_tree(ret_arr[idx]);
		free(ret_arr);
		free(used);
	}

	if (*retained) {
		(*retained)->first_child = new_first;
		(*retained)->last_child = new_last;
		(*retained)->type = declared->type;
		(*retained)->layout_opts = declared->layout_opts;
		(*retained)->label_text = declared->label_text;
		(*retained)->button_id = declared->button_id;
		(*retained)->aria_label = declared->aria_label;
		(*retained)->tab_index = declared->tab_index;
		(*retained)->text_color = declared->text_color;
		(*retained)->text_decoration = declared->text_decoration;
		(*retained)->scroll_max_h = declared->scroll_max_h;
		(*retained)->spacer_w = declared->spacer_w;
		(*retained)->spacer_h = declared->spacer_h;
		(*retained)->icon_id = declared->icon_id;
		copy_state_retained_to_declared(declared, *retained);
	} else {
		cui_node *root = clone_retained_from_declared(declared);
		if (root) {
			root->first_child = new_first;
			root->last_child = new_last;
			copy_state_retained_to_declared(declared, root);
			*retained = root;
		}
	}
}

void cui_diff_run(cui_node *declared, cui_node **retained) {
	if (!retained) return;
	if (!declared) {
		cui_diff_free_retained(retained);
		return;
	}
	if (!*retained) {
		cui_node *root = clone_retained_from_declared(declared);
		if (!root) return;
		*retained = root;
		reconcile(declared, retained);
		copy_state_retained_to_declared(declared, *retained);
		return;
	}
	reconcile(declared, retained);
}
