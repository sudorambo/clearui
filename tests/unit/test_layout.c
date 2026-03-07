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
	b->type = CUI_NODE_LABEL;
	cui_node_append_child(root, a);
	cui_node_append_child(root, b);

	cui_layout_run(root, 400, 300);

	/* Column: a then b; a at top, b below with gap; both centered in 400 */
	assert(a->layout_x >= 150 && a->layout_x <= 160);
	assert(a->layout_y >= 0 && a->layout_y <= 1);
	assert(a->layout_w == 100 && a->layout_h == 20);

	assert(b->layout_x >= 150 && b->layout_x <= 160);
	assert(b->layout_y >= 28 && b->layout_y <= 30); /* 20 + gap 8 */
	assert(b->layout_w == 100 && b->layout_h == 20);

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

	cui_arena_free(&arena);
	printf("test_layout: PASS\n");
	return 0;
}
