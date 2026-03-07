#ifndef CLEARUI_FONT_ATLAS_H
#define CLEARUI_FONT_ATLAS_H

/**
 * Font atlas: default font id for layout/draw.
 * Full SDF atlas build (e.g. Noto Sans) and optional HarfBuzz for complex scripts
 * are follow-ups; see build docs and spec §4.
 */
int cui_font_default_id(void);

#endif
