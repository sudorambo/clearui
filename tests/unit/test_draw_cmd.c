#include "../../src/core/draw_cmd.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_draw_command_buffer buf;
	cui_draw_buf_init(&buf, 4);
	assert(buf.cmd != NULL);
	assert(buf.capacity == 4);
	assert(buf.count == 0);

	assert(cui_draw_buf_push_rect(&buf, 10, 20, 30, 40, 0xff0000ffu) == 0);
	assert(buf.count == 1);
	assert(buf.cmd[0].type == CUI_CMD_RECT);

	assert(cui_draw_buf_push_line(&buf, 0, 0, 100, 100, 2.f, 0xff00ff00u) == 0);
	assert(buf.count == 2);
	assert(buf.cmd[1].type == CUI_CMD_LINE);

	assert(cui_draw_buf_push_text(&buf, 5, 5, "hello", 0xff000000u) == 0);
	assert(buf.count == 3);
	assert(buf.cmd[2].type == CUI_CMD_TEXT);

	assert(cui_draw_buf_push_rounded_rect(&buf, 0, 0, 50, 50, 10, 0xffff0000u) == 0);
	assert(buf.count == 4);
	assert(buf.cmd[3].type == CUI_CMD_ROUNDED_RECT);
	assert(buf.cmd[3].u.rounded_rect.r == 10.f);

	assert(cui_draw_buf_push_rect(&buf, 0, 0, 1, 1, 0xffu) == -1);
	assert(buf.count == 4);

	cui_draw_buf_clear(&buf);
	assert(buf.count == 0);

	assert(cui_draw_buf_push_scissor(&buf, 10, 20, 100, 200) == 0);
	assert(buf.count == 1);
	assert(buf.cmd[0].type == CUI_CMD_SCISSOR);

	assert(cui_draw_buf_push_text(&buf, 0, 0, NULL, 0) == -1);

	cui_draw_buf_fini(&buf);
	assert(buf.cmd == NULL && buf.capacity == 0);

	printf("test_draw_cmd: PASS\n");
	return 0;
}
