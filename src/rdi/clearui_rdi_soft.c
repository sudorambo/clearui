/**
 * Software RDI: rasterize draw command buffer to an RGBA framebuffer.
 * Contract: specs/021-render-driver-0-8-0/contracts/rdi-software-0.8.md
 */
#include "../../include/clearui_rdi.h"
#include "../../include/clearui_platform.h"
#include "../core/draw_cmd.h"
#include "../core/utf8.h"
#include "../font/atlas.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct soft_ctx {
	unsigned char *fb;
	int            width;
	int            height;
	int            scissor_x;
	int            scissor_y;
	int            scissor_w;
	int            scissor_h;
	void                  *platform_ctx;
	const struct cui_platform *platform;
} soft_ctx;

static int init(cui_rdi_context **out_ctx) {
	soft_ctx *c = (soft_ctx *)malloc(sizeof(soft_ctx));
	if (!c) return -1;
	memset(c, 0, sizeof(soft_ctx));
	*out_ctx = (cui_rdi_context *)c;
	return 0;
}

static void shutdown(cui_rdi_context *ctx) {
	if (!ctx) return;
	soft_ctx *c = (soft_ctx *)ctx;
	free(c->fb);
	c->fb = NULL;
	c->width = c->height = 0;
	free(ctx);
}

void cui_rdi_soft_set_viewport(cui_rdi_context *ctx, int width, int height) {
	if (!ctx || width <= 0 || height <= 0) return;
	soft_ctx *c = (soft_ctx *)ctx;
	if (c->width == width && c->height == height) return;
	free(c->fb);
	c->fb = (unsigned char *)malloc((size_t)width * (size_t)height * 4);
	c->width = width;
	c->height = height;
	if (!c->fb) c->width = c->height = 0;
}

void cui_rdi_soft_set_platform(cui_rdi_context *ctx, void *platform_ctx, const void *platform) {
	if (!ctx) return;
	soft_ctx *c = (soft_ctx *)ctx;
	c->platform_ctx = platform_ctx;
	c->platform = (const struct cui_platform *)platform;
}

void cui_rdi_soft_get_framebuffer(cui_rdi_context *ctx, const void **out_rgba, int *out_width, int *out_height) {
	if (!ctx) return;
	soft_ctx *c = (soft_ctx *)ctx;
	if (out_rgba) *out_rgba = c->fb;
	if (out_width) *out_width = c->width;
	if (out_height) *out_height = c->height;
}

static void scissor(cui_rdi_context *ctx, int x, int y, int w, int h) {
	if (!ctx) return;
	soft_ctx *c = (soft_ctx *)ctx;
	c->scissor_x = x;
	c->scissor_y = y;
	c->scissor_w = w;
	c->scissor_h = h;
}

/* Intersect draw rect (x,y,w,h) with current scissor; output in *ox,*oy,*ow,*oh. Return 0 if no overlap. */
static int clip_to_scissor(const soft_ctx *c, int x, int y, int w, int h,
                           int *ox, int *oy, int *ow, int *oh) {
	int sx = c->scissor_x;
	int sy = c->scissor_y;
	int sw = c->scissor_w;
	int sh = c->scissor_h;
	int x2 = x + w;
	int y2 = y + h;
	int sx2 = sx + sw;
	int sy2 = sy + sh;
	int cx = x > sx ? x : sx;
	int cy = y > sy ? y : sy;
	int cx2 = x2 < sx2 ? x2 : sx2;
	int cy2 = y2 < sy2 ? y2 : sy2;
	if (cx >= cx2 || cy >= cy2) return 0;
	*ox = cx;
	*oy = cy;
	*ow = cx2 - cx;
	*oh = cy2 - cy;
	return 1;
}

static void fill_rect(soft_ctx *c, int x, int y, int w, int h, unsigned int color) {
	int ox, oy, ow, oh;
	if (!clip_to_scissor(c, x, y, w, h, &ox, &oy, &ow, &oh) || !c->fb) return;
	const int pitch = c->width * 4;
	unsigned char r = (unsigned char)(color >> 16);
	unsigned char g = (unsigned char)(color >> 8);
	unsigned char b = (unsigned char)(color);
	unsigned char a = (unsigned char)(color >> 24);
	for (int row = 0; row < oh; row++) {
		unsigned char *p = c->fb + (oy + row) * pitch + ox * 4;
		for (int col = 0; col < ow; col++) {
			p[0] = r;
			p[1] = g;
			p[2] = b;
			p[3] = a;
			p += 4;
		}
	}
}

/* 1 if (px,py) is inside rounded rect (rx,ry,rw,rh) with radius r; 0 otherwise. */
static int inside_rounded_rect(int px, int py, int rx, int ry, int rw, int rh, int r) {
	if (r <= 0 || rw <= 0 || rh <= 0) return 0;
	if (px < rx || px >= rx + rw || py < ry || py >= ry + rh) return 0;
	if (r > rw / 2) r = rw / 2;
	if (r > rh / 2) r = rh / 2;
	int cx1 = rx + r, cy1 = ry + r;
	int cx2 = rx + rw - r, cy2 = ry + rh - r;
	if (px >= cx1 && px < cx2) return 1;
	if (py >= cy1 && py < cy2) return 1;
	int dx, dy;
	if (px < cx1 && py < cy1) { dx = px - cx1; dy = py - cy1; }
	else if (px >= cx2 && py < cy1) { dx = px - cx2; dy = py - cy1; }
	else if (px < cx1 && py >= cy2) { dx = px - cx1; dy = py - cy2; }
	else { dx = px - cx2; dy = py - cy2; }
	return (dx * dx + dy * dy) <= (r * r);
}

static void fill_rounded_rect(soft_ctx *c, int rx, int ry, int rw, int rh, int r, unsigned int color) {
	int ox, oy, ow, oh;
	if (!clip_to_scissor(c, rx, ry, rw, rh, &ox, &oy, &ow, &oh) || !c->fb) return;
	unsigned char cr = (unsigned char)(color >> 16);
	unsigned char cg = (unsigned char)(color >> 8);
	unsigned char cb = (unsigned char)(color);
	unsigned char ca = (unsigned char)(color >> 24);
	const int pitch = c->width * 4;
	for (int row = 0; row < oh; row++) {
		int py = oy + row;
		unsigned char *p = c->fb + py * pitch + ox * 4;
		for (int col = 0; col < ow; col++) {
			int px = ox + col;
			if (inside_rounded_rect(px, py, rx, ry, rw, rh, r)) {
				p[0] = cr; p[1] = cg; p[2] = cb; p[3] = ca;
			}
			p += 4;
		}
	}
}

/* Bresenham line from (x0,y0) to (x1,y1); draw each pixel as 1px (thickness handled by multiple lines). */
static void draw_line_1px(soft_ctx *c, int x0, int y0, int x1, int y1, unsigned int color) {
	int dx = x1 - x0, dy = y1 - y0;
	int ax = (dx < 0 ? -dx : dx), ay = (dy < 0 ? -dy : dy);
	int sx = dx < 0 ? -1 : 1, sy = dy < 0 ? -1 : 1;
	int x = x0, y = y0;
	unsigned char r = (unsigned char)(color >> 16);
	unsigned char g = (unsigned char)(color >> 8);
	unsigned char b = (unsigned char)(color);
	unsigned char a = (unsigned char)(color >> 24);
	const int pitch = c->width * 4;
	if (ax > ay) {
		int err = 2 * ay - ax;
		for (int i = 0; i <= ax; i++) {
			if (x >= 0 && x < c->width && y >= 0 && y < c->height) {
				int ox, oy, ow, oh;
				if (clip_to_scissor(c, x, y, 1, 1, &ox, &oy, &ow, &oh)) {
					unsigned char *p = c->fb + oy * pitch + ox * 4;
					p[0] = r; p[1] = g; p[2] = b; p[3] = a;
				}
			}
			if (err >= 0) { y += sy; err -= 2 * ax; }
			err += 2 * ay;
			x += sx;
		}
	} else {
		int err = 2 * ax - ay;
		for (int i = 0; i <= ay; i++) {
			if (x >= 0 && x < c->width && y >= 0 && y < c->height) {
				int ox, oy, ow, oh;
				if (clip_to_scissor(c, x, y, 1, 1, &ox, &oy, &ow, &oh)) {
					unsigned char *p = c->fb + oy * pitch + ox * 4;
					p[0] = r; p[1] = g; p[2] = b; p[3] = a;
				}
			}
			if (err >= 0) { x += sx; err -= 2 * ay; }
			err += 2 * ax;
			y += sy;
		}
	}
}

static void draw_line(soft_ctx *c, float x0, float y0, float x1, float y1, float thickness, unsigned int color) {
	int ix0 = (int)x0, iy0 = (int)y0, ix1 = (int)x1, iy1 = (int)y1;
	int t = (int)(thickness + 0.5f);
	if (t < 1) t = 1;
	float dx = x1 - x0, dy = y1 - y0;
	double len_sq = (double)(dx * dx + dy * dy);
	if (len_sq <= 0) { draw_line_1px(c, ix0, iy0, ix1, iy1, color); return; }
	float len = (float)sqrt(len_sq);
	float perp_x = -dy / len, perp_y = dx / len;
	for (int k = -t / 2; k <= (t + 1) / 2; k++) {
		int ox0 = ix0 + (int)(k * perp_x);
		int oy0 = iy0 + (int)(k * perp_y);
		int ox1 = ix1 + (int)(k * perp_x);
		int oy1 = iy1 + (int)(k * perp_y);
		draw_line_1px(c, ox0, oy0, ox1, oy1, color);
	}
}

#define CUI_SOFT_DEFAULT_FONT_SIZE 16

static void draw_text(soft_ctx *c, float fx, float fy, const char *text, unsigned int color) {
	if (!text || !c->fb) return;
	unsigned char cr = (unsigned char)(color >> 16);
	unsigned char cg = (unsigned char)(color >> 8);
	unsigned char cb = (unsigned char)(color);
	unsigned char ca = (unsigned char)(color >> 24);
	int x = (int)fx, y = (int)fy;
	const int pitch = c->width * 4;
	const unsigned char *p = (const unsigned char *)text;
	int font_id = cui_font_default_id();
	int font_size = CUI_SOFT_DEFAULT_FONT_SIZE;
	while (*p) {
		int cp;
		int adv = cui_utf8_next(p, &cp);
		if (adv <= 0) break;
		unsigned char *bitmap = NULL;
		int gw, gh, gx0, gy0, advance;
		if (!cui_font_render_glyph(font_id, font_size, cp, &bitmap, &gw, &gh, &gx0, &gy0, &advance)) {
			p += adv;
			continue;
		}
		int dst_x0 = x + gx0;
		int dst_y0 = y + gy0;
		for (int row = 0; row < gh; row++) {
			int dy = dst_y0 + row;
			if (dy < 0 || dy >= c->height) continue;
			for (int col = 0; col < gw; col++) {
				int dx = dst_x0 + col;
				if (dx < 0 || dx >= c->width) continue;
				int ox, oy, ow, oh;
				if (!clip_to_scissor(c, dx, dy, 1, 1, &ox, &oy, &ow, &oh)) continue;
				unsigned char alpha = bitmap[row * gw + col];
				unsigned char *dst = c->fb + oy * pitch + ox * 4;
				if (alpha == 0) continue;
				if (alpha >= 253) {
					dst[0] = cr; dst[1] = cg; dst[2] = cb; dst[3] = ca;
				} else {
					unsigned int inv = 255 - alpha;
					dst[0] = (unsigned char)((dst[0] * inv + cr * alpha) / 255);
					dst[1] = (unsigned char)((dst[1] * inv + cg * alpha) / 255);
					dst[2] = (unsigned char)((dst[2] * inv + cb * alpha) / 255);
					dst[3] = (unsigned char)((dst[3] * inv + ca * alpha) / 255);
				}
			}
		}
		cui_font_free_glyph_bitmap(bitmap);
		x += advance;
		p += adv;
	}
}

static int submit(cui_rdi_context *ctx, const cui_draw_command_buffer *buf) {
	if (!ctx || !buf) return 0;
	soft_ctx *c = (soft_ctx *)ctx;
	if (!c->fb || c->width <= 0 || c->height <= 0) return 0;
	memset(c->fb, 0, (size_t)c->width * (size_t)c->height * 4);
	c->scissor_x = 0;
	c->scissor_y = 0;
	c->scissor_w = c->width;
	c->scissor_h = c->height;

	for (size_t i = 0; i < buf->count; i++) {
		const cui_draw_cmd *cmd = &buf->cmd[i];
		switch (cmd->type) {
		case CUI_CMD_SCISSOR: {
			int x = (int)cmd->u.scissor.x;
			int y = (int)cmd->u.scissor.y;
			int w = (int)cmd->u.scissor.w;
			int h = (int)cmd->u.scissor.h;
			if (w < 0) { x += w; w = -w; }
			if (h < 0) { y += h; h = -h; }
			c->scissor_x = x < 0 ? 0 : x;
			c->scissor_y = y < 0 ? 0 : y;
			c->scissor_w = (x + w > c->width ? c->width - c->scissor_x : w);
			c->scissor_h = (y + h > c->height ? c->height - c->scissor_y : h);
			if (c->scissor_w < 0) c->scissor_w = 0;
			if (c->scissor_h < 0) c->scissor_h = 0;
			break;
		}
		case CUI_CMD_RECT: {
			int x = (int)cmd->u.fill_rect.x;
			int y = (int)cmd->u.fill_rect.y;
			int w = (int)cmd->u.fill_rect.w;
			int h = (int)cmd->u.fill_rect.h;
			if (w < 0) { x += w; w = -w; }
			if (h < 0) { y += h; h = -h; }
			fill_rect(c, x, y, w, h, cmd->u.fill_rect.color);
			break;
		}
		case CUI_CMD_ROUNDED_RECT: {
			int rx = (int)cmd->u.rounded_rect.x;
			int ry = (int)cmd->u.rounded_rect.y;
			int rw = (int)cmd->u.rounded_rect.w;
			int rh = (int)cmd->u.rounded_rect.h;
			int r = (int)(cmd->u.rounded_rect.r + 0.5f);
			if (rw < 0) { rx += rw; rw = -rw; }
			if (rh < 0) { ry += rh; rh = -rh; }
			fill_rounded_rect(c, rx, ry, rw, rh, r, cmd->u.rounded_rect.color);
			break;
		}
		case CUI_CMD_LINE:
			draw_line(c, cmd->u.line.a.x, cmd->u.line.a.y, cmd->u.line.b.x, cmd->u.line.b.y,
			          cmd->u.line.thickness, cmd->u.line.color);
			break;
		case CUI_CMD_TEXT:
			draw_text(c, cmd->u.text.x, cmd->u.text.y, cmd->u.text.text, cmd->u.text.color);
			break;
		case CUI_CMD_NOP:
		default:
			break;
		}
	}
	return 0;
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

static int present(cui_rdi_context *ctx) {
	if (!ctx) return 0;
	soft_ctx *c = (soft_ctx *)ctx;
	if (c->platform && c->platform->present_software && c->platform_ctx && c->fb && c->width > 0 && c->height > 0)
		c->platform->present_software(c->platform_ctx, c->fb, c->width, c->height, c->width * 4);
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
