/**
 * Unit test: cui_stack places children with overlapping layout rects (z-order).
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/node.h"
#include <assert.h>
#include <stdio.h>

static cui_node *find_first_stack(cui_node *n) {
	if (!n) return NULL;
	if (n->type == CUI_NODE_STACK) return n;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) {
		cui_node *f = find_first_stack(c);
		if (f) return f;
	}
	return NULL;
}

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "stack", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "stack", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_stack(ctx, &(cui_layout){ .gap = 0 });
	cui_spacer(ctx, 40, 40);
	cui_spacer(ctx, 40, 40);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_node *root = cui_ctx_declared_root(ctx);
	assert(root != NULL);
	cui_node *stack = find_first_stack(root);
	assert(stack != NULL && stack->type == CUI_NODE_STACK);
	cui_node *a = stack->first_child;
	cui_node *b = a ? a->next_sibling : NULL;
	assert(a != NULL && b != NULL && "stack should have two children");
	/* Stack places all children at same (layout_x, layout_y) */
	assert(a->layout_x >= b->layout_x - 0.01f && a->layout_x <= b->layout_x + 0.01f);
	assert(a->layout_y >= b->layout_y - 0.01f && a->layout_y <= b->layout_y + 0.01f);

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_stack: PASS\n");
	return 0;
}
