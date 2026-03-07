/**
 * Integration test: RDI + platform (stub) init, one frame submit, shutdown.
 * Complements T019 / test_hello by asserting RDI and platform contract.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	assert(plat != NULL && rdi != NULL);

	cui_platform_ctx *plat_ctx = NULL;
	int r = plat->window_create(&plat_ctx, "RDI test", 320, 200);
	assert(r == 0 && plat_ctx != NULL);

	cui_rdi_context *rdi_ctx = NULL;
	r = rdi->init(&rdi_ctx);
	assert(r == 0 && rdi_ctx != NULL);

	cui_ctx *ctx = cui_create(&(cui_config){ .title = "RDI", .width = 320, .height = 200 });
	assert(ctx != NULL);
	cui_set_platform(ctx, plat, plat_ctx);
	cui_set_rdi(ctx, rdi, rdi_ctx);

	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_label(ctx, "RDI+Platform");
	cui_end(ctx);
	cui_end_frame(ctx);

	rdi->shutdown(rdi_ctx);
	plat->window_destroy(plat_ctx);
	cui_destroy(ctx);

	printf("test_rdi_platform: PASS\n");
	return 0;
}
