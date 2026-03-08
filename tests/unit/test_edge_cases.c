/**
 * Unit test: NULL ctx and empty tree - no crash.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	/* NULL ctx: all should early-return without crashing */
	cui_begin_frame(NULL);
	cui_end_frame(NULL);
	cui_button(NULL, "id");
	cui_label(NULL, "text");
	cui_spacer(NULL, 0, 0);
	cui_center(NULL);
	cui_end(NULL);
	cui_push_style(NULL, NULL);
	cui_pop_style(NULL);
	cui_label_styled(NULL, "t", NULL);
	cui_row(NULL, NULL);
	cui_column(NULL, NULL);
	cui_stack(NULL, NULL);
	cui_wrap(NULL, NULL);
	cui_scroll(NULL, NULL, NULL);
	cui_canvas(NULL, NULL);
	cui_draw_rect(NULL, 0, 0, 10, 10, 0);
	cui_draw_circle(NULL, 0, 0, 5, 0);
	cui_draw_text(NULL, 0, 0, "x", 0);
	cui_inject_click(NULL, 0, 0);
	cui_inject_key(NULL, 0);
	cui_inject_char(NULL, 'a');
	cui_inject_mouse_move(NULL, 0, 0);
	cui_inject_scroll(NULL, 0, 0);

	/* Empty frame: begin_frame, end_frame with no widgets */
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "edge", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "edge", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	cui_begin_frame(ctx);
	cui_end_frame(ctx);

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_edge_cases: PASS\n");
	return 0;
}
