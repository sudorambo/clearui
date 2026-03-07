/**
 * Integration test: Hello World pipeline.
 * Context + stub platform + soft RDI; one frame with center + label.
 */
#include "clearui.h"
#include "clearui_platform.h"
#include "clearui_rdi.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){
		.title = "Hello",
		.width = 400,
		.height = 300,
	});
	assert(ctx != NULL);

	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	assert(plat && rdi);

	cui_platform_ctx *plat_ctx = NULL;
	int r = plat->window_create(&plat_ctx, "Hello", 400, 300);
	assert(r == 0 && plat_ctx != NULL);

	cui_rdi_context *rdi_ctx = NULL;
	r = rdi->init(&rdi_ctx);
	assert(r == 0 && rdi_ctx != NULL);

	cui_set_platform(ctx, plat, plat_ctx);
	cui_set_rdi(ctx, rdi, rdi_ctx);

	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_label(ctx, "Hello, World!");
	cui_end(ctx);
	cui_end_frame(ctx);

	assert(cui_running(ctx));

	rdi->shutdown(rdi_ctx);
	plat->window_destroy(plat_ctx);
	cui_destroy(ctx);

	printf("test_hello: PASS\n");
	return 0;
}
