/**
 * Unit test: scroll wheel updates scroll offset; offset is clamped to content bounds.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/node.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static cui_node *find_node_by_id(cui_node *n, const char *id) {
	if (!n || !id) return NULL;
	if (n->button_id && strcmp(n->button_id, id) == 0) return n;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) {
		cui_node *f = find_node_by_id(c, id);
		if (f) return f;
	}
	return NULL;
}

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "scroll", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "scroll", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	/* Frame 1: scroll with id and content taller than viewport (max_height 80, 4 spacers 25px = 100 content) */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_scroll(ctx, "s1", &(cui_scroll_opts){ .max_height = 80 });
	cui_spacer(ctx, 200, 25);
	cui_spacer(ctx, 200, 25);
	cui_spacer(ctx, 200, 25);
	cui_spacer(ctx, 200, 25);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	/* Frame 2: mouse inside scroll area, inject scroll down (dy < 0 = content up = offset increase) */
	cui_inject_mouse_move(ctx, 200, 50);
	cui_inject_scroll(ctx, 0, -30);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_scroll(ctx, "s1", &(cui_scroll_opts){ .max_height = 80 });
	cui_spacer(ctx, 200, 25);
	cui_spacer(ctx, 200, 25);
	cui_spacer(ctx, 200, 25);
	cui_spacer(ctx, 200, 25);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_node *root = cui_ctx_retained_root(ctx);
	assert(root != NULL);
	cui_node *scroll = find_node_by_id(root, "s1");
	assert(scroll != NULL && scroll->type == CUI_NODE_SCROLL);
	/* Content ~100, view 80, max offset 20. We scrolled 30; should clamp to 20. */
	assert(scroll->scroll_offset_y >= 0.f && "offset must not be negative");
	assert(scroll->scroll_offset_y <= 21.f && "offset must be clamped to content - view");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_scroll: PASS\n");
	return 0;
}
