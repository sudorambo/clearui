/**
 * Integration test: type into text field via inject API, run frame loop, assert buffer content.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "edit", .width = 400, .height = 300 });
	assert(ctx != NULL);

	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	assert(plat && rdi);
	cui_platform_ctx *plat_ctx = NULL;
	plat->window_create(&plat_ctx, "edit", 400, 300);
	cui_rdi_context *rdi_ctx = NULL;
	rdi->init(&rdi_ctx);
	cui_set_platform(ctx, plat, plat_ctx);
	cui_set_rdi(ctx, rdi, rdi_ctx);

	char buf[64] = {0};

	/* Frame 1: declare one text input */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "name", buf, sizeof(buf), &(cui_text_input_opts){ .placeholder = "Name" });
	cui_end(ctx);
	cui_end_frame(ctx);

	/* Type "Hi" (one char per frame) */
	cui_inject_char(ctx, 'H');
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "name", buf, sizeof(buf), NULL);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_inject_char(ctx, 'i');
	cui_begin_frame(ctx);
	cui_center(ctx);
	int changed = cui_text_input(ctx, "name", buf, sizeof(buf), NULL);
	cui_end(ctx);
	cui_end_frame(ctx);

	assert(strcmp(buf, "Hi") == 0 && "buffer should be Hi");
	assert(changed == 1 && "cui_text_input should return 1 after content changed");

	rdi->shutdown(rdi_ctx);
	plat->window_destroy(plat_ctx);
	cui_destroy(ctx);
	printf("test_text_input_edit: PASS\n");
	return 0;
}
