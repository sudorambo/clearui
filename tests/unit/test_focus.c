#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "focus", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "focus", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	/* Frame 1: two buttons */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_button(ctx, "A");
	cui_button(ctx, "B");
	cui_end(ctx);
	cui_end_frame(ctx);

	/* Frame 2: Tab → focus moves to first focusable */
	cui_inject_key(ctx, CUI_KEY_TAB);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_button(ctx, "A");
	cui_button(ctx, "B");
	cui_end(ctx);
	cui_end_frame(ctx);

	const char *fid = cui_ctx_focused_id(ctx);
	assert(fid != NULL);

	/* Frame 3: Tab again → focus moves to second */
	cui_inject_key(ctx, CUI_KEY_TAB);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_button(ctx, "A");
	cui_button(ctx, "B");
	cui_end(ctx);
	cui_end_frame(ctx);

	const char *fid2 = cui_ctx_focused_id(ctx);
	assert(fid2 != NULL);
	assert(strcmp(fid, fid2) != 0 && "Tab should cycle focus");

	/* Frame 4: Shift-Tab → focus moves back */
	cui_inject_key(ctx, CUI_KEY_SHIFT_TAB);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_button(ctx, "A");
	cui_button(ctx, "B");
	cui_end(ctx);
	cui_end_frame(ctx);

	const char *fid3 = cui_ctx_focused_id(ctx);
	assert(fid3 != NULL);
	assert(strcmp(fid3, fid) == 0 && "Shift-Tab should reverse focus");

	/* Frame 5: inject Enter to activate focused button */
	cui_inject_key(ctx, CUI_KEY_ENTER);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_button(ctx, "A");
	cui_button(ctx, "B");
	cui_end(ctx);
	cui_end_frame(ctx);

	/* Frame 6: consume the activation */
	cui_begin_frame(ctx);
	cui_center(ctx);
	int a_clicked = cui_button(ctx, "A");
	int b_clicked = cui_button(ctx, "B");
	cui_end(ctx);
	cui_end_frame(ctx);

	assert((a_clicked || b_clicked) && "Enter should activate focused button");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_focus: PASS\n");
	return 0;
}
