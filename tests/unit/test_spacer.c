/**
 * Unit test: cui_spacer yields node with layout_w/layout_h consistent with declared w, h.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/node.h"
#include <assert.h>
#include <stdio.h>

static cui_node *find_first_spacer(cui_node *n) {
	if (!n) return NULL;
	if (n->type == CUI_NODE_SPACER) return n;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) {
		cui_node *f = find_first_spacer(c);
		if (f) return f;
	}
	return NULL;
}

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "spacer", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "spacer", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	float w = 100.f, h = 50.f;
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_spacer(ctx, w, h);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_node *root = cui_ctx_declared_root(ctx);
	assert(root != NULL);
	cui_node *spacer = find_first_spacer(root);
	assert(spacer != NULL && spacer->type == CUI_NODE_SPACER);
	assert(spacer->spacer_w == w && spacer->spacer_h == h);
	assert(spacer->layout_w >= w - 1.f && spacer->layout_w <= w + 1.f && "layout_w should match spacer_w");
	assert(spacer->layout_h >= h - 1.f && spacer->layout_h <= h + 1.f && "layout_h should match spacer_h");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_spacer: PASS\n");
	return 0;
}
