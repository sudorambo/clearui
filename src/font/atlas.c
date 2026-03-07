/**
 * Font atlas: default font id and text measurement for layout/draw.
 * Uses vendored stb_truetype for metrics; optional HarfBuzz for complex scripts is a follow-up.
 */
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../deps/stb_truetype.h"
#include "atlas.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CUI_FONT_TTF_PATH "deps/default_font.ttf"
#define CUI_FONT_TTF_MAX (512 * 1024)

static stbtt_fontinfo s_font;
static unsigned char *s_ttf_buf;
static int s_inited; /* 0=not tried, 1=ok, -1=failed */

/* TTF magic: 0x00010000 (true) or 'OTTO' for OTF */
static int is_ttf_or_otf(const unsigned char *buf, size_t n) {
	if (n < 4) return 0;
	if (buf[0] == 0x00 && buf[1] == 0x01 && buf[2] == 0x00 && buf[3] == 0x00) return 1;
	if (buf[0] == 'O' && buf[1] == 'T' && buf[2] == 'T' && buf[3] == 'O') return 1;
	return 0;
}

static void init_font_once(void) {
	if (s_inited != 0) return;
	s_ttf_buf = (unsigned char *)malloc(CUI_FONT_TTF_MAX);
	if (!s_ttf_buf) { s_inited = -1; return; }
	FILE *f = fopen(CUI_FONT_TTF_PATH, "rb");
	if (!f) { free(s_ttf_buf); s_ttf_buf = NULL; s_inited = -1; return; }
	size_t n = fread(s_ttf_buf, 1, CUI_FONT_TTF_MAX, f);
	fclose(f);
	if (n < 4 || !is_ttf_or_otf(s_ttf_buf, n)) { free(s_ttf_buf); s_ttf_buf = NULL; s_inited = -1; return; }
	int offset = stbtt_GetFontOffsetForIndex(s_ttf_buf, 0);
	if (offset < 0 || !stbtt_InitFont(&s_font, s_ttf_buf, offset)) { free(s_ttf_buf); s_ttf_buf = NULL; s_inited = -1; return; }
	s_inited = 1;
}

/* Decode next UTF-8 codepoint; return byte advance (0 at end), codepoint in *out. */
static int utf8_next(const unsigned char *s, int *out_cp) {
	unsigned char c = s[0];
	if (c == 0) { *out_cp = 0; return 0; }
	if (c < 0x80) { *out_cp = (int)c; return 1; }
	if (c < 0xe0 && s[1]) { *out_cp = (int)(c & 0x1f) << 6 | (int)(s[1] & 0x3f); return 2; }
	if (c < 0xf0 && s[1] && s[2]) { *out_cp = (int)(c & 0x0f) << 12 | (int)(s[1] & 0x3f) << 6 | (int)(s[2] & 0x3f); return 3; }
	if (c < 0xf8 && s[1] && s[2] && s[3]) { *out_cp = (int)(c & 0x07) << 18 | (int)(s[1] & 0x3f) << 12 | (int)(s[2] & 0x3f) << 6 | (int)(s[3] & 0x3f); return 4; }
	*out_cp = (int)(c & 0x7f);
	return 1;
}

int cui_font_default_id(void) {
	return 0;
}

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
		int adv = utf8_next(p, &cp);
		if (adv == 0) break;
		int advance, lsb;
		stbtt_GetCodepointHMetrics(&s_font, cp, &advance, &lsb);
		width += scale * (float)advance;
		p += adv;
	}
	if (out_width) *out_width = width;
}
