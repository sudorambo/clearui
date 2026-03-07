/**
 * Unit test: text input character insertion, backspace, delete, buffer full.
 */
#include "../../include/clearui.h"
#include "../../include/clearui_platform.h"
#include "../../include/clearui_rdi.h"
#include "../../src/core/context.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
	cui_ctx *ctx = cui_create(&(cui_config){ .title = "text", .width = 400, .height = 300 });
	assert(ctx != NULL);
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();
	cui_platform_ctx *pc = NULL;
	cui_rdi_context *rc = NULL;
	plat->window_create(&pc, "text", 400, 300);
	rdi->init(&rc);
	cui_set_platform(ctx, plat, pc);
	cui_set_rdi(ctx, rdi, rc);

	char buf[64] = {0};
	const size_t cap = sizeof(buf);

	/* Delete at start (run first so cursor is 0): one text input, buffer "hello", inject Delete -> "ello" */
	{
		char hbuf[64];
		memcpy(hbuf, "hello", 6);
		cui_begin_frame(ctx);
		cui_center(ctx);
		cui_text_input(ctx, "h", hbuf, sizeof(hbuf), NULL);
		cui_end(ctx);
		cui_end_frame(ctx);
		cui_inject_key(ctx, CUI_KEY_DELETE);
		cui_begin_frame(ctx);
		cui_center(ctx);
		cui_text_input(ctx, "h", hbuf, sizeof(hbuf), NULL);
		cui_end(ctx);
		cui_end_frame(ctx);
		assert(strcmp(hbuf, "ello") == 0 && "delete at start");
	}

	/* Frame 1: declare one text input so it is the only focusable (focused by default) */
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "input", buf, cap, &(cui_text_input_opts){ .placeholder = "Type" });
	cui_end(ctx);
	cui_end_frame(ctx);

	/* Frame 2: inject 'a', run frame -> buffer "a" */
	cui_inject_char(ctx, 'a');
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "input", buf, cap, &(cui_text_input_opts){ .placeholder = "Type" });
	cui_end(ctx);
	cui_end_frame(ctx);
	assert(strcmp(buf, "a") == 0);

	/* Frame 3: inject 'b', 'c' (one per frame) -> "abc" */
	cui_inject_char(ctx, 'b');
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "input", buf, cap, NULL);
	cui_end(ctx);
	cui_end_frame(ctx);
	cui_inject_char(ctx, 'c');
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "input", buf, cap, NULL);
	cui_end(ctx);
	cui_end_frame(ctx);
	assert(strcmp(buf, "abc") == 0);

	/* Frame 4: backspace -> "ab" */
	cui_inject_key(ctx, CUI_KEY_BACKSPACE);
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "input", buf, cap, NULL);
	cui_end(ctx);
	cui_end_frame(ctx);
	assert(strcmp(buf, "ab") == 0);

	/* Buffer full: fill to cap-1, one more insert should be no-op */
	memset(buf, 0, cap);
	buf[0] = 'x';
	for (size_t i = 1; i < cap - 1; i++) buf[i] = 'y';
	buf[cap - 1] = '\0';
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "input", buf, cap, NULL);
	cui_end(ctx);
	cui_end_frame(ctx);
	cui_inject_char(ctx, 'z');
	cui_begin_frame(ctx);
	cui_center(ctx);
	cui_text_input(ctx, "input", buf, cap, NULL);
	cui_end(ctx);
	cui_end_frame(ctx);
	assert(buf[cap - 2] == 'y' && buf[cap - 1] == '\0' && "full buffer: insert no-op");


	rdi->shutdown(rc);
	plat->window_destroy(pc);
	cui_destroy(ctx);
	printf("test_text_input: PASS\n");
	return 0;
}
