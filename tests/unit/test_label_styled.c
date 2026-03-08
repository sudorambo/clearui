/**
 * Unit test: cui_label_styled applies text_color (and optional text_decoration) to the label node.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/node.h"
#include <assert.h>
#include <stdio.h>

static cui_node *find_first_label(cui_node *n) {
	if (!n) return NULL;
	if (n->type == CUI_NODE_LABEL) return n;
	for (cui_node *c = n->first_child; c; c = c->next_sibling) {
		cui_node *f = find_first_label(c);
		if (f) return f;
	}
	return NULL;
}

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "label_styled", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "label_styled", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	unsigned int want_color = 0xff0080ff;
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_label_styled(ctx, "Styled", &(cui_style){ .text_color = want_color });
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_node *root = cui_ctx_declared_root(ctx);
	assert(root != NULL);
	cui_node *label = find_first_label(root);
	assert(label != NULL && label->type == CUI_NODE_LABEL);
	assert(label->text_color == want_color && "label_styled should set text_color on node");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_label_styled: PASS\n");
	return 0;
}
