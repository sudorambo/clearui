/**
 * Unit test: Hi-DPI scale_buf path - coordinates scaled by scale_factor after end_frame.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/draw_cmd.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){
		.title = "scale_buf",
		.width = 400,
		.height = 300,
		.scale_factor = 2.f
	});
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "scale_buf", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	/* Produce a draw command with known logical coords (e.g. canvas rect at 10,10) */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_canvas(ctx, &(cui_canvas_opts){ .width = 100, .height = 100 });
	cui_draw_rect(ctx, 10.f, 10.f, 20.f, 20.f, 0xff0000ff);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	/* render_submit (called in end_frame) copies draw_buf -> scaled_buf with scale 2 */
	cui_draw_command_buffer *buf = cui_ctx_draw_buf(ctx);
	cui_draw_command_buffer *scaled = cui_ctx_scaled_buf(ctx);
	assert(buf != NULL && scaled != NULL && buf->count > 0 && scaled->count == buf->count);
	/* Same command at index i should have coords scaled by 2 */
	int found = 0;
	for (size_t i = 0; i < buf->count && i < scaled->count; i++) {
		if (buf->cmd[i].type == CUI_CMD_RECT && scaled->cmd[i].type == CUI_CMD_RECT) {
			float lx = buf->cmd[i].u.fill_rect.x, ly = buf->cmd[i].u.fill_rect.y;
			float sx = scaled->cmd[i].u.fill_rect.x, sy = scaled->cmd[i].u.fill_rect.y;
			if (sx >= lx * 1.9f && sx <= lx * 2.1f && sy >= ly * 1.9f && sy <= ly * 2.1f)
				found = 1;
			break;
		}
	}
	assert(found && "scaled_buf coordinates should be scale_factor * logical");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_scale_buf: PASS\n");
	return 0;
}
