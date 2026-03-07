/**
 * Counter example: cui_state, cui_frame_printf, cui_button.
 * One frame with inject_click on "+" button; next frame count should be 1.
 */
#include "clearui.h"
#include "clearui_platform.h"
#include "clearui_rdi.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "Counter", .width = 320, .height = 200 });
	assert(ctx != NULL);

	cui_platform_ctx *plat_ctx = NULL;
	assert(cui_platform_stub_get()->window_create(&plat_ctx, "Counter", 320, 200) == 0);
	cui_rdi_context *rdi_ctx = NULL;
	assert(cui_rdi_soft_get()->init(&rdi_ctx) == 0);
	cui_set_platform(ctx, cui_platform_stub_get(), plat_ctx);
	cui_set_rdi(ctx, cui_rdi_soft_get(), rdi_ctx);

	int *count = (int *)cui_state(ctx, "counter", sizeof(int));
	assert(count != NULL && *count == 0);

	/* Frame 1: build UI and inject click on "+" button (laid out at ~160,60) */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_label(ctx, cui_frame_printf(ctx, "Count: %d", *count));
	cui_button(ctx, "-");
	cui_button(ctx, "+");
	cui_inject_click(ctx, 200, 70);
	cui_end(ctx);
	cui_end_frame(ctx);

	/* Frame 2: consume click in cui_button("+") and increment */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_label(ctx, cui_frame_printf(ctx, "Count: %d", *count));
	if (cui_button(ctx, "-")) (*count)--;
	if (cui_button(ctx, "+")) (*count)++;
	cui_end(ctx);
	cui_end_frame(ctx);

	assert(*count == 1 && "counter state should persist and increment on + click");

	cui_rdi_soft_get()->shutdown(rdi_ctx);
	cui_platform_stub_get()->window_destroy(plat_ctx);
	cui_destroy(ctx);
	printf("test_counter: PASS\n");
	return 0;
}
