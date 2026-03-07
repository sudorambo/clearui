/**
 * UI tree nodes: arena-allocated, intrusive linked-list children.
 * Nodes are allocated from the per-frame arena and never individually freed.
 * The tree is rebuilt every frame; the diff engine reconciles with retained state.
 */
#include "node.h"
#include "arena.h"
#include <string.h>

void cui_node_tree_clear(cui_node **root) {
	*root = NULL;
}

cui_node *cui_node_alloc(cui_arena *arena) {
	cui_node *n = (cui_node *)cui_arena_alloc(arena, sizeof(cui_node));
	if (!n) return NULL;
	memset(n, 0, sizeof(cui_node));
	return n;
}

void cui_node_append_child(cui_node *parent, cui_node *child) {
	if (!parent || !child) return;
	child->next_sibling = NULL;
	if (!parent->first_child) {
		parent->first_child = child;
	} else {
		parent->last_child->next_sibling = child;
	}
	parent->last_child = child;
}
