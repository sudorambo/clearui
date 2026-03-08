/**
 * Unit test: cui_set_theme affects draw command colors; cui_set_theme(ctx, NULL) resets to default.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include "../../src/core/draw_cmd.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "theme", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "theme", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	/* Custom theme: distinct button background so we can find it in draw buffer */
	cui_theme custom;
	custom.text_color = 0xff000000u;
	custom.button_bg = 0xff112233u;
	custom.checkbox_bg = 0xffffffffu;
	custom.input_bg = 0xffffffffu;
	custom.corner_radius = 4.f;
	custom.font_size = 16;
	custom.focus_ring_color = 0xff0066ccu;
	custom.focus_ring_width = 2.f;
	cui_set_theme(ctx, &custom);

	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_button(ctx, "btn");
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	cui_draw_command_buffer *buf = cui_ctx_draw_buf(ctx);
	assert(buf != NULL);
	int found_custom_bg = 0;
	for (size_t i = 0; i < buf->count; i++) {
		if (buf->cmd[i].type == CUI_CMD_RECT &&
		    buf->cmd[i].u.fill_rect.color == 0xff112233u) {
			found_custom_bg = 1;
			break;
		}
	}
	assert(found_custom_bg && "draw_buf should contain button rect with custom theme button_bg");

	/* Reset to default; next frame should use default button_bg (0xffe0e0e0) */
	cui_set_theme(ctx, NULL);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_button(ctx, "btn2");
	cui_end(ctx);
	cui_end(ctx);
	cui_end_frame(ctx);

	buf = cui_ctx_draw_buf(ctx);
	assert(buf != NULL);
	int found_default_bg = 0;
	for (size_t i = 0; i < buf->count; i++) {
		if (buf->cmd[i].type == CUI_CMD_RECT &&
		    buf->cmd[i].u.fill_rect.color == 0xffe0e0e0u) {
			found_default_bg = 1;
			break;
		}
	}
	assert(found_default_bg && "after cui_set_theme(ctx, NULL), draw should use default button_bg");

	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_theme: PASS\n");
	return 0;
}
