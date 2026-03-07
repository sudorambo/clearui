/**
 * Unit test for layout engine: row/column with gap and align.
 */
#include "core/arena.h"
#include "core/node.h"
#include "layout/layout.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_arena arena;
	cui_arena_init(&arena, 65536);

	cui_node *root = cui_node_alloc(&arena);
	assert(root != NULL);
	root->type = CUI_NODE_CENTER;
	root->layout_w = 400;
	root->layout_h = 300;

	cui_node *a = cui_node_alloc(&arena);
	cui_node *b = cui_node_alloc(&arena);
	assert(a != NULL && b != NULL);
	a->type = CUI_NODE_LABEL;
	a->label_text = "Label A";
	b->type = CUI_NODE_LABEL;
	b->label_text = "Label B";
	cui_node_append_child(root, a);
	cui_node_append_child(root, b);

	cui_layout_run(root, 400, 300);

	/* Column: a then b; a at top, b below with gap; both centered in 400 */
	assert(a->layout_w > 0 && a->layout_h > 0);
	assert(b->layout_w > 0 && b->layout_h > 0);
	assert(a->layout_x >= 100 && a->layout_x <= 200);
	assert(a->layout_y >= 0 && a->layout_y <= 10);
	assert(b->layout_y >= a->layout_y + a->layout_h);

	/* Row: build row with two buttons, check horizontal layout */
	cui_arena_reset(&arena);
	cui_node *row = cui_node_alloc(&arena);
	assert(row != NULL);
	row->type = CUI_NODE_ROW;
	row->layout_opts.gap = 12;
	cui_node *b1 = cui_node_alloc(&arena);
	cui_node *b2 = cui_node_alloc(&arena);
	b1->type = CUI_NODE_BUTTON;
	b2->type = CUI_NODE_BUTTON;
	cui_node_append_child(row, b1);
	cui_node_append_child(row, b2);

	cui_layout_run(row, 200, 50);

	assert(b1->layout_x >= 0 && b1->layout_y >= 0);
	assert(b1->layout_w == 80 && b1->layout_h == 24);
	assert(b2->layout_x >= 92 && b2->layout_x <= 95); /* 80 + gap 12 */
	assert(b2->layout_y == b1->layout_y);

	/* T003: last_child invariants */
	cui_arena_reset(&arena);
	{
		cui_node *p = cui_node_alloc(&arena);
		assert(p != NULL);
		p->type = CUI_NODE_COLUMN;

		cui_node *ca = cui_node_alloc(&arena);
		ca->type = CUI_NODE_LABEL;
		cui_node_append_child(p, ca);
		assert(p->first_child == ca);
		assert(p->last_child == ca);

		cui_node *cb = cui_node_alloc(&arena);
		cb->type = CUI_NODE_LABEL;
		cui_node_append_child(p, cb);
		assert(p->first_child == ca);
		assert(p->last_child == cb);
		assert(ca->next_sibling == cb);

		cui_node *cc = cui_node_alloc(&arena);
		cc->type = CUI_NODE_LABEL;
		cui_node_append_child(p, cc);
		assert(p->first_child == ca);
		assert(p->last_child == cc);
		assert(ca->next_sibling == cb);
		assert(cb->next_sibling == cc);
		assert(cc->next_sibling == NULL);
	}

	/* Content-aware label sizing: two labels with different text get different layout_w */
	cui_arena_reset(&arena);
	{
		cui_node *row = cui_node_alloc(&arena);
		assert(row != NULL);
		row->type = CUI_NODE_ROW;
		row->layout_w = 400;
		row->layout_h = 50;
		cui_node *short_label = cui_node_alloc(&arena);
		cui_node *long_label = cui_node_alloc(&arena);
		assert(short_label != NULL && long_label != NULL);
		short_label->type = CUI_NODE_LABEL;
		short_label->label_text = "Short";
		long_label->type = CUI_NODE_LABEL;
		long_label->label_text = "Much longer text";
		cui_node_append_child(row, short_label);
		cui_node_append_child(row, long_label);
		cui_layout_run(row, 400, 50);
		assert(short_label->layout_w < long_label->layout_w && "content-aware sizing: longer text should have larger width");
	}

	cui_arena_free(&arena);
	printf("test_layout: PASS\n");
	return 0;
}
