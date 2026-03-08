/**
 * Unit test: cui_push_style / cui_pop_style nesting and restoration.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "style_stack", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "style_stack", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	unsigned int color_a = 0x11111111;
	unsigned int color_b = 0x22222222;
	cui_begin_frame(ctx);
	cui_push_style(ctx, &(cui_style){ .text_color = color_a });
	cui_push_style(ctx, &(cui_style){ .text_color = color_b });
	cui_pop_style(ctx);
	const cui_style *cur = cui_ctx_current_style(ctx);
	assert(cur != NULL && cur->text_color == color_a && "pop should restore style A");

	cui_pop_style(ctx);
	cur = cui_ctx_current_style(ctx);
	/* After second pop we're back to default (0 or previous); just ensure no crash */
	(void)cur;
	cui_end_frame(ctx);

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_style_stack: PASS\n");
	return 0;
}
