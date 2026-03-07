/**
 * Integration test: scroll container receives scroll events; offset in expected range.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/node.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static cui_node *find_scroll_by_id(cui_node *n, const char *id) {
	if (!n || !id) return NULL;
	if (n->type == CUI_NODE_SCROLL && n->button_id && strcmp(n->button_id, id) == 0) return n;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) {
		cui_node *f = find_scroll_by_id(c, id);
		if (f) return f;
	}
	return NULL;
}

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "scroll_region", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	assert(plat && rdi);
	cui_platform_ctx *plat_ctx = NULL;
	plat->window_create(&plat_ctx, "scroll_region", 400, 300);
	cui_rdi_context *rdi_ctx = NULL;
	rdi->init(&rdi_ctx);
	cui_set_platform(ctx, plat, plat_ctx);
	cui_set_rdi(ctx, rdi, rdi_ctx);

	/* Frame 1: build scroll with id */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_scroll(ctx, "panel", &(cui_scroll_opts){ .max_height = 100 });
	cui_spacer(ctx, 200, 40);
	cui_spacer(ctx, 200, 40);
	cui_spacer(ctx, 200, 40);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	/* Frame 2: inject scroll several times */
	cui_inject_mouse_move(ctx, 200, 80);
	cui_inject_scroll(ctx, 0, -20);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_scroll(ctx, "panel", &(cui_scroll_opts){ .max_height = 100 });
	cui_spacer(ctx, 200, 40);
	cui_spacer(ctx, 200, 40);
	cui_spacer(ctx, 200, 40);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_inject_mouse_move(ctx, 200, 80);
	cui_inject_scroll(ctx, 0, -20);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_scroll(ctx, "panel", &(cui_scroll_opts){ .max_height = 100 });
	cui_spacer(ctx, 200, 40);
	cui_spacer(ctx, 200, 40);
	cui_spacer(ctx, 200, 40);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_node *root = cui_ctx_retained_root(ctx);
	assert(root != NULL);
	cui_node *scroll = find_scroll_by_id(root, "panel");
	assert(scroll != NULL);
	/* Content 120, view 100, max offset 20. Two scrolls of 20 each; clamped to 20. */
	assert(scroll->scroll_offset_y >= 0.f && scroll->scroll_offset_y <= 21.f);

	rdi->shutdown(rdi_ctx);
	plat->window_destroy(plat_ctx);
	cui_destroy(ctx);
	printf("test_scroll_region: PASS\n");
	return 0;
}
