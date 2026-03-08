#ifndef CLEARUI_FONT_ATLAS_H
#define CLEARUI_FONT_ATLAS_H

/**
 * Font atlas: default font id and text measurement for layout/draw.
 * Full SDF atlas and optional HarfBuzz for complex scripts are follow-ups.
 */
int cui_font_default_id(void);

/**
 * Measure a UTF-8 string for the given font and size.
 * Fills out_width (advance) and out_height (line height) in logical pixels.
 * If utf8 is NULL or empty: out_width = 0, out_height = line height for font_size_px.
 */
void cui_font_measure(int font_id, int font_size_px, const char *utf8, float *out_width, float *out_height);

/**
 * Rasterize one glyph to an 8-bit alpha bitmap. Used by software RDI for text.
 * Returns 1 on success, 0 if font not loaded or glyph missing. *out_bitmap is allocated
 * by stb_truetype; caller must free with cui_font_free_glyph_bitmap.
 * out_advance is in pixels (scaled).
 */
int cui_font_render_glyph(int font_id, int font_size_px, int codepoint,
                          unsigned char **out_bitmap, int *out_w, int *out_h,
                          int *out_x0, int *out_y0, int *out_advance);
/** Free bitmap returned by cui_font_render_glyph. */
void cui_font_free_glyph_bitmap(unsigned char *bitmap);

#endif
