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

	/* Events: poll and return whether app should keep running.
	 * Second arg is cui_ctx* (void* to avoid circular include); adapter may call
	 * cui_inject_mouse_move, cui_inject_click, etc. NULL when not set. */
	bool (*poll_events)(cui_platform_ctx *ctx, void *cui_ctx);

	/* Optional: NULL means not supported */
	int (*clipboard_get)(cui_platform_ctx *ctx, char *buf, size_t size);
	int (*clipboard_set)(cui_platform_ctx *ctx, const char *text);
	void (*cursor_set)(cui_platform_ctx *ctx, int shape);

	/* Hi-DPI: return scale factor (>= 1.0); NULL = not supported. */
	float (*scale_factor_get)(cui_platform_ctx *ctx);

	/* GPU surface for RDI (e.g. swapchain); NULL if not used */
	void *(*surface_get)(cui_platform_ctx *ctx);
	void (*surface_destroy)(cui_platform_ctx *ctx, void *surface);
} cui_platform;

/* Cursor shapes for cursor_set; adapter may map to platform-specific values. */
#define CUI_CURSOR_DEFAULT 0
#define CUI_CURSOR_TEXT    1

const cui_platform *cui_platform_stub_get(void);

#ifdef __cplusplus
}
#endif

#endif /* CLEARUI_PLATFORM_H */
