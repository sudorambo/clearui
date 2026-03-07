/**
 * ClearUI Render Driver Interface (RDI)
 * Contract: specs/001-clearui-core/contracts/rdi-interface.md
 * ClearUI calls these function pointers; one implementation (e.g. software) per build.
 */
#ifndef CLEARUI_RDI_H
#define CLEARUI_RDI_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque types for RDI internal state */
typedef struct cui_rdi_context cui_rdi_context;
typedef struct cui_rdi_texture cui_rdi_texture;

/* Command buffer produced by Core; RDI consumes it (tagged union in draw_cmd.h) */
typedef struct cui_draw_command_buffer cui_draw_command_buffer;

typedef struct cui_rdi {
	/* Init / shutdown */
	int (*init)(cui_rdi_context **out_ctx);
	void (*shutdown)(cui_rdi_context *ctx);

	/* Texture */
	int (*texture_create)(cui_rdi_context *ctx, int width, int height, const void *rgba, cui_rdi_texture **out_tex);
	int (*texture_upload)(cui_rdi_context *ctx, cui_rdi_texture *tex, int x, int y, int w, int h, const void *rgba);
	void (*texture_destroy)(cui_rdi_context *ctx, cui_rdi_texture *tex);

	/* Draw: submit command buffer for this frame */
	int (*submit)(cui_rdi_context *ctx, const cui_draw_command_buffer *buf);
	void (*scissor)(cui_rdi_context *ctx, int x, int y, int w, int h);
	int (*present)(cui_rdi_context *ctx);
} cui_rdi;

const cui_rdi *cui_rdi_soft_get(void);

#ifdef __cplusplus
}
#endif

#endif /* CLEARUI_RDI_H */
