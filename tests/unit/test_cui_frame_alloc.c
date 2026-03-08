/**
 * Unit test: cui_frame_alloc lifetime - allocation valid only until next cui_begin_frame.
 * We do not dereference the old pointer after begin_frame; test passes under ASan.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "cui_frame_alloc", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "cui_frame_alloc", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	cui_begin_frame(ctx);
	void *p1 = cui_frame_alloc(ctx, 64);
	assert(p1 != NULL);
	memset(p1, 0xab, 64);
	cui_end_frame(ctx);

	cui_begin_frame(ctx);
	/* Do not use p1 here; allocator was reset. Use a new allocation in this frame. */
	void *p2 = cui_frame_alloc(ctx, 32);
	assert(p2 != NULL);
	memset(p2, 0xcd, 32);
	cui_end_frame(ctx);

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_cui_frame_alloc: PASS\n");
	return 0;
}
