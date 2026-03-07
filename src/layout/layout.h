#ifndef CLEARUI_LAYOUT_H
#define CLEARUI_LAYOUT_H

#include "../core/node.h"

/**
 * Layout engine: single pass + flex-grow over node tree.
 * Sets layout_x, layout_y, layout_w, layout_h (logical pixels) on each node.
 * Call with viewport size; root container (e.g. CENTER) gets that size if not set.
 */
void cui_layout_run(cui_node *root, float viewport_w, float viewport_h);

#endif
