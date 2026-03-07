#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/a11y.h"
#include "../../src/core/node.h"
#include "../../src/core/arena.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "a11y", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "a11y", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	/* Frame 1: checkbox (unchecked) + button — build a11y tree */
	int checked = 0;
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_checkbox(ctx, "chk1", &checked);
	cui_button(ctx, "btn1");
	cui_end(ctx);
	cui_end_frame(ctx);

	const cui_a11y_tree *tree = cui_ctx_a11y_tree(ctx);
	assert(tree != NULL && tree->count >= 2);
	int found_chk = 0, found_btn = 0;
	for (int i = 0; i < tree->count; i++) {
		if (tree->entry[i].role == CUI_A11Y_ROLE_CHECKBOX) {
			assert(strstr(tree->entry[i].state, "checked") == NULL);
			found_chk = 1;
		}
		if (tree->entry[i].role == CUI_A11Y_ROLE_BUTTON) {
			found_btn = 1;
		}
	}
	assert(found_chk && found_btn);

	/* Frame 2: checkbox (checked) + button, inject Tab to focus checkbox */
	checked = 1;
	cui_inject_key(ctx, CUI_KEY_TAB);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_checkbox(ctx, "chk1", &checked);
	cui_button(ctx, "btn1");
	cui_end(ctx);
	cui_end_frame(ctx);

	tree = cui_ctx_a11y_tree(ctx);
	assert(tree != NULL);
	for (int i = 0; i < tree->count; i++) {
		if (tree->entry[i].role == CUI_A11Y_ROLE_CHECKBOX) {
			assert(strstr(tree->entry[i].state, "checked") != NULL);
		}
	}

	/* Label fallback: aria_label > label_text > button_id */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_aria_label(ctx, "custom-label");
	cui_button(ctx, "btn2");
	cui_end(ctx);
	cui_end_frame(ctx);

	tree = cui_ctx_a11y_tree(ctx);
	int found_custom = 0;
	for (int i = 0; i < tree->count; i++) {
		if (tree->entry[i].role == CUI_A11Y_ROLE_BUTTON &&
		    strcmp(tree->entry[i].label, "custom-label") == 0) {
			found_custom = 1;
		}
	}
	assert(found_custom && "aria_label should override button_id");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_a11y: PASS\n");
	return 0;
}
