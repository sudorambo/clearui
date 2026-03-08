/**
 * Integration test: open real window (SDL3), one frame, close.
 * Skip with "SKIP (no display or SDL3)" when window creation fails (e.g. headless CI).
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform_sdl3.h"
#include "../../include/clearui_rdi.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
	const cui_platform *platform = cui_platform_sdl3_get();
	if (!platform) {
		printf("test_platform_window: SKIP (no display or SDL3)\n");
		return 0;
	}

	cui_platform_ctx *platform_ctx = NULL;
	if (platform->window_create(&platform_ctx, "test_platform_window", 400, 300) != 0 || !platform_ctx) {
		printf("test_platform_window: SKIP (no display or SDL3)\n");
		return 0;
	}

	cui_ctx *ctx = cui_create(&(cui_config){ .title = "test", .width = 400, .height = 300 });
	if (!ctx) {
		platform->window_destroy(platform_ctx);
		printf("test_platform_window: FAIL (cui_create)\n");
		return 1;
	}

	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_rdi_context *rdi_ctx = NULL;
	if (rdi) rdi->init(&rdi_ctx);
	cui_set_platform(ctx, platform, platform_ctx);
	cui_set_rdi(ctx, rdi, rdi_ctx);

	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_label(ctx, "OK");
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	platform->window_destroy(platform_ctx);
	if (rdi && rdi_ctx) rdi->shutdown(rdi_ctx);
	cui_destroy(ctx);

	printf("test_platform_window: PASS\n");
	return 0;
}
