/**
 * ClearUI Platform Adapter Interface
 * Contract: specs/001-clearui-core/contracts/platform-interface.md
 * App provides or ClearUI uses built-in (e.g. SDL3). App owns event loop.
 */
#ifndef CLEARUI_PLATFORM_H
#define CLEARUI_PLATFORM_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cui_platform_ctx cui_platform_ctx;

typedef struct cui_platform {
	/* Window */
	int (*window_create)(cui_platform_ctx **out_ctx, const char *title, int width, int height);
	void (*window_destroy)(cui_platform_ctx *ctx);
	void (*window_get_size)(cui_platform_ctx *ctx, int *width, int *height);

	/* Events: poll and return whether app should keep running */
	bool (*poll_events)(cui_platform_ctx *ctx);

	/* Optional: NULL means not supported */
	int (*clipboard_get)(cui_platform_ctx *ctx, char *buf, size_t size);
	int (*clipboard_set)(cui_platform_ctx *ctx, const char *text);
	void (*cursor_set)(cui_platform_ctx *ctx, int shape);

	/* GPU surface for RDI (e.g. swapchain); NULL if not used */
	void *(*surface_get)(cui_platform_ctx *ctx);
	void (*surface_destroy)(cui_platform_ctx *ctx, void *surface);
} cui_platform;

const cui_platform *cui_platform_stub_get(void);

#ifdef __cplusplus
}
#endif

#endif /* CLEARUI_PLATFORM_H */
