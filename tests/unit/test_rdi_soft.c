/**
 * Unit test: software RDI rasterizes draw commands to framebuffer.
 * Pushes a fullscreen rect, submits, and checks that the framebuffer has non-zero pixels.
 * To add a golden checksum baseline: run once, print the sum, then assert equality (update when rendering changes).
 */
#include "../../include/clearui_rdi.h"
#include "../../src/core/draw_cmd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
	cui_draw_command_buffer buf;
	cui_draw_buf_init(&buf, 8);
	assert(buf.cmd != NULL && buf.capacity >= 8);
	cui_draw_buf_push_rect(&buf, 0.f, 0.f, 64.f, 64.f, 0xff0000ffu);
	assert(buf.count == 1);

	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_rdi_context *rc = NULL;
	assert(rdi->init(&rc) == 0 && rc != NULL);
	cui_rdi_soft_set_viewport(rc, 64, 64);
	rdi->submit(rc, &buf);

	const void *rgba = NULL;
	int w = 0, h = 0;
	cui_rdi_soft_get_framebuffer(rc, &rgba, &w, &h);
	assert(rgba != NULL && w == 64 && h == 64);
	unsigned int sum = 0;
	const unsigned char *p = (const unsigned char *)rgba;
	for (int i = 0; i < w * h * 4; i++)
		sum += p[i];
	assert(sum != 0 && "software RDI should have drawn something");

	cui_draw_buf_fini(&buf);
	rdi->shutdown(rc);
	printf("test_rdi_soft: PASS\n");
	return 0;
}
