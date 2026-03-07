/**
 * Unit test: draw buffer configurable capacity.
 * Uses internal headers to assert capacity after create.
 */
#include "clearui.h"
#include "core/context.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	/* Default: draw_buf_capacity 0 => capacity 1024 */
	{
		cui_config config = {
			.title = "Default",
			.width = 400,
			.height = 300,
			.scale_factor = 1.f,
			.draw_buf_capacity = 0,
		};
		cui_ctx *ctx = cui_create(&config);
		assert(ctx != NULL);
		cui_draw_command_buffer *buf = cui_ctx_draw_buf(ctx);
		assert(buf != NULL && buf->cmd != NULL);
		assert(buf->capacity == 1024 && "default capacity should be 1024");
		cui_destroy(ctx);
	}
	/* Explicit 2048 */
	{
		cui_config config = {
			.title = "Custom",
			.width = 400,
			.height = 300,
			.scale_factor = 1.f,
			.draw_buf_capacity = 2048,
		};
		cui_ctx *ctx = cui_create(&config);
		assert(ctx != NULL);
		cui_draw_command_buffer *buf = cui_ctx_draw_buf(ctx);
		assert(buf != NULL && buf->cmd != NULL);
		assert(buf->capacity == 2048 && "custom capacity should be 2048");
		cui_destroy(ctx);
	}
	printf("test_draw_buf: PASS\n");
	return 0;
}
