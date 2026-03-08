/**
 * Stress test: 1000+ widgets per frame, multiple frames. No crash, acceptable time.
 */
#include "../../include/clearui.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_config config = { 0 };
	config.width = 400;
	config.height = 600;
	cui_ctx *ctx = cui_create(&config);
	assert(ctx && "cui_create");

	for (int frame = 0; frame < 10; frame++) {
		cui_begin_frame(ctx);
		cui_column(ctx, &(cui_layout){ .gap = 2 });
		for (int i = 0; i < 1200; i++) {
			char id[32];
			snprintf(id, sizeof(id), "btn%d", i);
			cui_button(ctx, id);
		}
		cui_end(ctx);
		cui_end_frame(ctx);
	}

	cui_destroy(ctx);
	return 0;
}
