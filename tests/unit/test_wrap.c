/**
 * Unit test: cui_wrap positions children in a flow (multiple rows when they overflow).
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/node.h"
#include <assert.h>
#include <stdio.h>

static cui_node *find_first_wrap(cui_node *n) {
	if (!n) return NULL;
	if (n->type == CUI_NODE_WRAP) return n;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) {
		cui_node *f = find_first_wrap(c);
		if (f) return f;
	}
	return NULL;
}

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "wrap", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "wrap", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	/* Wrap with max_width so children wrap to multiple rows */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_wrap(ctx, &(cui_layout){ .gap = 4, .max_width = 100 });
	cui_spacer(ctx, 60, 20);
	cui_spacer(ctx, 60, 20);
	cui_spacer(ctx, 60, 20);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_node *root = cui_ctx_declared_root(ctx);
	assert(root != NULL);
	cui_node *wrap = find_first_wrap(root);
	assert(wrap != NULL && wrap->type == CUI_NODE_WRAP);
	/* Wrap has 3 spacers; with narrow width at least two rows (different layout_y) */
	float first_y = -1.f;
	int different_y = 0;
	for (cui_node *c = wrap->first_child; c; c = c->next_sibling) {
		if (first_y < 0) first_y = c->layout_y;
		else if (c->layout_y > first_y + 1.f) different_y = 1;
	}
	assert(different_y && "wrap should place some children on a different row");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_wrap: PASS\n");
	return 0;
}
