/**
 * Headless platform stub: no window, poll_events always returns true.
 * For CI and tests. Use SDL3 adapter for real window.
 */
#include "clearui_platform.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int width, height; } stub_ctx;

static int window_create(cui_platform_ctx **out_ctx, const char *title, int width, int height) {
	(void)title;
	stub_ctx *c = (stub_ctx *)malloc(sizeof(stub_ctx));
	if (!c) return -1;
	c->width = width;
	c->height = height;
	*out_ctx = (cui_platform_ctx *)c;
	return 0;
}

static void window_destroy(cui_platform_ctx *ctx) {
	free(ctx);
}

static void window_get_size(cui_platform_ctx *ctx, int *width, int *height) {
	stub_ctx *c = (stub_ctx *)ctx;
	if (width) *width = c ? c->width : 0;
	if (height) *height = c ? c->height : 0;
}

static bool poll_events(cui_platform_ctx *ctx) {
	(void)ctx;
	return true;
}

static cui_platform stub_platform = {
	.window_create = window_create,
	.window_destroy = window_destroy,
	.window_get_size = window_get_size,
	.poll_events = poll_events,
	.clipboard_get = NULL,
	.clipboard_set = NULL,
	.cursor_set = NULL,
	.surface_get = NULL,
	.surface_destroy = NULL,
};

const cui_platform *cui_platform_stub_get(void) {
	return &stub_platform;
}
