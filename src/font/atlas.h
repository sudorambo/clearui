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

#endif
