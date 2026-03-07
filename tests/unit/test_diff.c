/**
 * Unit test: diff engine — retained tree structure and state preservation.
 * Builds a small declared tree, runs cui_diff_run twice, asserts retained
 * is non-NULL and scroll state is copied from retained to declared.
 */
#include "../../src/core/diff.h"
#include "../../src/core/node.h"
#include "../../src/core/arena.h"
#include <assert.h>
#include <stdio.h>

static int count_children(cui_node *n) {
	int k = 0;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) k++;
	return k;
}

int main(void) {
	cui_arena arena;
	cui_arena_init(&arena, 64 * 1024);
	cui_node *declared = cui_node_alloc(&arena);
	assert(declared != NULL);
	declared->type = CUI_NODE_CENTER;
	declared->first_child = declared->last_child = declared->next_sibling = NULL;

	cui_node *scroll = cui_node_alloc(&arena);
	assert(scroll != NULL);
	scroll->type = CUI_NODE_SCROLL;
	scroll->scroll_offset_y = 0.f;
	scroll->first_child = scroll->last_child = scroll->next_sibling = NULL;
	cui_node_append_child(declared, scroll);

	cui_node *retained = NULL;
	cui_diff_run(declared, &retained);
	assert(retained != NULL && "retained root should be created");
	assert(retained->type == CUI_NODE_CENTER);
	assert(count_children(retained) == 1);
	assert(retained->first_child != NULL && retained->first_child->type == CUI_NODE_SCROLL);

	/* Simulate user scroll: retain state in retained tree */
	retained->first_child->scroll_offset_y = 50.f;

	/* Second "frame": new declared tree (same structure), diff again */
	cui_arena_reset(&arena);
	cui_node *declared2 = cui_node_alloc(&arena);
	assert(declared2 != NULL);
	declared2->type = CUI_NODE_CENTER;
	declared2->first_child = declared2->last_child = declared2->next_sibling = NULL;
	cui_node *scroll2 = cui_node_alloc(&arena);
	assert(scroll2 != NULL);
	scroll2->type = CUI_NODE_SCROLL;
	scroll2->scroll_offset_y = 0.f;
	scroll2->first_child = scroll2->last_child = scroll2->next_sibling = NULL;
	cui_node_append_child(declared2, scroll2);

	cui_diff_run(declared2, &retained);
	assert(retained != NULL);
	/* State should be copied from retained to declared */
	assert(scroll2->scroll_offset_y >= 49.f && scroll2->scroll_offset_y <= 51.f && "scroll state should be preserved");

	cui_diff_free_retained(&retained);
	assert(retained == NULL);
	cui_arena_free(&arena);
	printf("test_diff: PASS\n");
	return 0;
}
