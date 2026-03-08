/**
 * Unit test: cui_canvas, cui_draw_rect, cui_draw_circle, cui_draw_text produce
 * expected draw buffer contents (replayed into main draw_buf after end_frame).
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/draw_cmd.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "canvas_draw", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "canvas_draw", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_canvas(ctx, &(cui_canvas_opts){ .width = 200, .height = 200 });
	cui_draw_rect(ctx, 10.f, 10.f, 50.f, 30.f, 0xff0000ff);
	cui_draw_circle(ctx, 100.f, 100.f, 20.f, 0x00ff00ff);
	cui_draw_text(ctx, 5.f, 5.f, "Hi", 0x0000ffff);
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_draw_command_buffer *buf = cui_ctx_draw_buf(ctx);
	assert(buf != NULL && buf->count >= 3);
	int has_rect = 0, has_rounded = 0, has_text = 0;
	for (size_t i = 0; i < buf->count; i++) {
		switch (buf->cmd[i].type) {
		case CUI_CMD_RECT:
			if (buf->cmd[i].u.fill_rect.w >= 49.f && buf->cmd[i].u.fill_rect.h >= 29.f &&
			    buf->cmd[i].u.fill_rect.color == 0xff0000ff)
				has_rect = 1;
			break;
		case CUI_CMD_ROUNDED_RECT:
			if (buf->cmd[i].u.rounded_rect.color == 0x00ff00ff) has_rounded = 1;
			break;
		case CUI_CMD_TEXT:
			if (buf->cmd[i].u.text.text && buf->cmd[i].u.text.color == 0x0000ffff) has_text = 1;
			break;
		default:
			break;
		}
	}
	assert(has_rect && "draw_buf should contain rect from cui_draw_rect");
	assert(has_rounded && "draw_buf should contain rounded_rect from cui_draw_circle");
	assert(has_text && "draw_buf should contain text from cui_draw_text");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_canvas_draw: PASS\n");
	return 0;
}
