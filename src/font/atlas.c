/**
 * Font atlas stub: expose default font id for layout/draw.
 * Full implementation: SDF atlas (e.g. Noto Sans, stb_truetype + distance field).
 * Build: optional HarfBuzz for complex-script shaping; see README when atlas is implemented.
 */
#include "font/atlas.h"

int cui_font_default_id(void) {
	return 0;
}
