#include "clearui_rdi.h"
#include "core/draw_cmd.h"
#include <stdlib.h>
#include <string.h>

typedef struct cui_rdi_context {
	int dummy;
} soft_ctx;

static int init(cui_rdi_context **out_ctx) {
	soft_ctx *c = (soft_ctx *)malloc(sizeof(soft_ctx));
	if (!c) return -1;
	memset(c, 0, sizeof(soft_ctx));
	*out_ctx = (cui_rdi_context *)c;
	return 0;
}

static void shutdown(cui_rdi_context *ctx) {
	free(ctx);
}

static int texture_create(cui_rdi_context *ctx, int width, int height, const void *rgba, cui_rdi_texture **out_tex) {
	(void)ctx; (void)width; (void)height; (void)rgba; (void)out_tex;
	return 0;
}

static int texture_upload(cui_rdi_context *ctx, cui_rdi_texture *tex, int x, int y, int w, int h, const void *rgba) {
	(void)ctx; (void)tex; (void)x; (void)y; (void)w; (void)h; (void)rgba;
	return 0;
}

static void texture_destroy(cui_rdi_context *ctx, cui_rdi_texture *tex) {
	(void)ctx; (void)tex;
}

/* Consumes same buffer format as GPU RDI; text commands use (x,y,text,color). Glyph/atlas UV wired when font atlas is implemented. */
static int submit(cui_rdi_context *ctx, const cui_draw_command_buffer *buf) {
	(void)ctx;
	if (buf) (void)buf->count;
	return 0;
}

static void scissor(cui_rdi_context *ctx, int x, int y, int w, int h) {
	(void)ctx; (void)x; (void)y; (void)w; (void)h;
}

static int present(cui_rdi_context *ctx) {
	(void)ctx;
	return 0;
}

static cui_rdi soft_rdi = {
	.init = init,
	.shutdown = shutdown,
	.texture_create = texture_create,
	.texture_upload = texture_upload,
	.texture_destroy = texture_destroy,
	.submit = submit,
	.scissor = scissor,
	.present = present,
};

const cui_rdi *cui_rdi_soft_get(void) {
	return &soft_rdi;
}
