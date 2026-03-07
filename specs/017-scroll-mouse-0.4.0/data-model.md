# Data Model: Scroll & Mouse (0.4.0)

## Existing entities (unchanged or extended)

### cui_node (scroll)

- `scroll_offset_y` (float) — already present; vertical scroll offset in logical pixels.
- `scroll_max_h` (float) — already present; visible viewport height.
- **Optional**: No new field required if we add scroll **id** via widget layer (e.g. `cui_scroll` takes an id and sets `button_id` on the node so scroll can be looked up). If we use `button_id` for scroll id, no node change.

### cui_ctx (extensions)

- **mouse_x**, **mouse_y** (int) — current pointer position in logical pixels; set by `cui_inject_mouse_move`; used for hover hit-test and scroll target.
- **hovered_id** (char[CUI_LAST_CLICKED_ID_MAX]) — id of the widget under the pointer after hover hit-test (depth-first, last wins). Cleared or updated each frame after hover hit-test.
- **pending_scroll_dx**, **pending_scroll_dy** (int or float) — pending scroll delta for this frame; applied in end_frame to the scroll node under the mouse; then cleared.

## Invariants

- **Scroll offset**: After applying scroll, `scroll_offset_y` is in `[0, max(0, content_height - scroll_max_h)]`.
- **Hover**: `hovered_id` is the id of the topmost (deepest) interactive widget whose layout rect contains (mouse_x, mouse_y), or empty if none.

## State transitions

- **inject_mouse_move(x, y)**: Set mouse_x, mouse_y.
- **inject_scroll(dx, dy)**: Set pending_scroll_dx, pending_scroll_dy (accumulate or replace: replace is simpler, one scroll per frame).
- **end_frame (after layout)**:
  1. Hover hit-test: walk declared tree with (mouse_x, mouse_y), set hovered_id.
  2. Scroll hit-test: find scroll node (with id) under (mouse_x, mouse_y); get its retained node; add pending_scroll_dy to scroll_offset_y; clamp; clear pending_scroll_*.
  3. (Optional) Clear or leave hovered_id for next frame; typically leave so app can read it until next inject_mouse_move.

## Content height for clamping

- For `CUI_NODE_SCROLL`, content height = height of the scroll’s content. In the current layout, the scroll node has one logical “content” child (or multiple); use the laid-out height of the scroll’s content (e.g. sum of children’s layout_h, or the scroll’s inner content height if already computed). Layout in `src/layout/layout.c` already handles SCROLL; we may need to expose or compute content height for clamping (e.g. from children after layout).
