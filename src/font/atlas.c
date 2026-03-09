/**
 * Font atlas: default font id and text measurement for layout/draw.
 * Uses vendored stb_truetype for metrics; optional HarfBuzz for complex scripts is a follow-up.
 */
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../deps/stb_truetype.h"
#include "atlas.h"
#include "core/utf8.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef CLEARUI_EMBED_FONT
#include "default_font_embed.h"
#endif

#define CUI_FONT_TTF_PATH_DEFAULT "deps/default_font.ttf"
#define CUI_FONT_TTF_MAX (512 * 1024)

static stbtt_fontinfo s_font;
static unsigned char *s_ttf_buf;
static int s_inited; /* 0=not tried, 1=ok, -1=failed */
static char s_font_path[1024];
static int  s_font_path_set;

/* TTF magic: 0x00010000 (true) or 'OTTO' for OTF */
static int is_ttf_or_otf(const unsigned char *buf, size_t n) {
	if (n < 4) return 0;
	if (buf[0] == 0x00 && buf[1] == 0x01 && buf[2] == 0x00 && buf[3] == 0x00) return 1;
	if (buf[0] == 'O' && buf[1] == 'T' && buf[2] == 'T' && buf[3] == 'O') return 1;
	return 0;
}

static int init_from_buffer(const unsigned char *data, size_t len) {
	if (len < 4 || !is_ttf_or_otf(data, len)) return -1;
	int offset = stbtt_GetFontOffsetForIndex(data, 0);
	if (offset < 0 || !stbtt_InitFont(&s_font, data, offset)) return -1;
	return 0;
}

void cui_set_font_path(const char *path) {
	if (path) {
		size_t n = strlen(path);
		if (n >= sizeof(s_font_path)) n = sizeof(s_font_path) - 1;
		memcpy(s_font_path, path, n);
		s_font_path[n] = '\0';
		s_font_path_set = 1;
	} else {
		s_font_path[0] = '\0';
		s_font_path_set = 0;
	}
}

int cui_load_font_memory(const void *data, size_t len) {
	if (s_inited != 0) return -1;
	if (!data || len < 4) { s_inited = -1; return -1; }
	s_ttf_buf = (unsigned char *)malloc(len);
	if (!s_ttf_buf) { s_inited = -1; return -1; }
	memcpy(s_ttf_buf, data, len);
	if (init_from_buffer(s_ttf_buf, len) != 0) {
		free(s_ttf_buf); s_ttf_buf = NULL; s_inited = -1; return -1;
	}
	s_inited = 1;
	return 0;
}

static void init_font_once(void) {
	if (s_inited != 0) return;

#ifdef CLEARUI_EMBED_FONT
	(void)cui_load_font_memory(clearui_default_font_ttf, clearui_default_font_ttf_len);
	if (s_inited == 1) return;
	s_inited = 0; /* fall through to file path */
#endif

	const char *path = s_font_path_set ? s_font_path : CUI_FONT_TTF_PATH_DEFAULT;
	s_ttf_buf = (unsigned char *)malloc(CUI_FONT_TTF_MAX);
	if (!s_ttf_buf) { s_inited = -1; return; }
	FILE *f = fopen(path, "rb");
	if (!f) { free(s_ttf_buf); s_ttf_buf = NULL; s_inited = -1; return; }
	size_t n = fread(s_ttf_buf, 1, CUI_FONT_TTF_MAX, f);
	fclose(f);
	if (init_from_buffer(s_ttf_buf, n) != 0) {
		free(s_ttf_buf); s_ttf_buf = NULL; s_inited = -1; return;
	}
	s_inited = 1;
}

int cui_font_default_id(void) {
	return 0;
}

/* utf8 must be a valid C string (NUL-terminated, valid UTF-8). */
void cui_font_measure(int font_id, int font_size_px, const char *utf8, float *out_width, float *out_height) {
	if (out_width) *out_width = 0.f;
	if (out_height) *out_height = font_size_px > 0 ? (float)font_size_px : 16.f;

	if (font_id != 0 || font_size_px <= 0) return;
	if (!utf8) return;

	init_font_once();
	if (s_inited != 1) {
		/* Fallback: approximate width by char count, theme-like height */
		size_t len = strlen(utf8);
		if (out_width) *out_width = 6.f * (float)len;
		if (out_height) *out_height = (float)font_size_px;
		return;
	}

	float scale = stbtt_ScaleForPixelHeight(&s_font, (float)font_size_px);
	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&s_font, &ascent, &descent, &line_gap);
	if (out_height) *out_height = scale * (float)(ascent - descent);

	if (utf8[0] == '\0') return;

	float width = 0.f;
	const unsigned char *p = (const unsigned char *)utf8;
	for (;;) {
		int cp;
		int adv = cui_utf8_next(p, &cp);
		if (adv == 0) break;
		int advance, lsb;
		stbtt_GetCodepointHMetrics(&s_font, cp, &advance, &lsb);
		width += scale * (float)advance;
		p += adv;
	}
	if (out_width) *out_width = width;
}

int cui_font_render_glyph(int font_id, int font_size_px, int codepoint,
                          unsigned char **out_bitmap, int *out_w, int *out_h,
                          int *out_x0, int *out_y0, int *out_advance) {
	if (out_bitmap) *out_bitmap = NULL;
	if (out_w) *out_w = 0;
	if (out_h) *out_h = 0;
	if (out_x0) *out_x0 = 0;
	if (out_y0) *out_y0 = 0;
	if (out_advance) *out_advance = 0;
	if (font_id != 0 || font_size_px <= 0 || !out_bitmap) return 0;
	init_font_once();
	if (s_inited != 1) return 0;
	float scale = stbtt_ScaleForPixelHeight(&s_font, (float)font_size_px);
	int advance, lsb;
	stbtt_GetCodepointHMetrics(&s_font, codepoint, &advance, &lsb);
	int w, h, x0, y0;
	unsigned char *bitmap = stbtt_GetCodepointBitmap(&s_font, 0, scale, codepoint, &w, &h, &x0, &y0);
	if (!bitmap) return 0;
	*out_bitmap = bitmap;
	if (out_w) *out_w = w;
	if (out_h) *out_h = h;
	if (out_x0) *out_x0 = x0;
	if (out_y0) *out_y0 = y0;
	if (out_advance) *out_advance = (int)(scale * (float)advance);
	return 1;
}

void cui_font_free_glyph_bitmap(unsigned char *bitmap) {
	if (bitmap) stbtt_FreeBitmap(bitmap, NULL);
}
