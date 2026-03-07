# Contract: Scroll & Hover API (0.4.0)

## New API

### cui_inject_scroll

```c
void cui_inject_scroll(cui_ctx *ctx, int dx, int dy);
```

- **Semantics**: Injects a scroll wheel event. `dx`, `dy` are in logical pixels; typically `dy < 0` means “scroll down” (content moves up, offset increases). Processed in `cui_end_frame`: the scroll container **under the current mouse position** (from `cui_inject_mouse_move`) receives the delta. Its `scroll_offset_y` is updated and clamped to content bounds. If no scroll container is under the mouse (or no scroll has an id), the event is ignored.
- **Target**: The scroll node under (mouse_x, mouse_y). Scroll nodes must have an id (e.g. `cui_scroll(ctx, "panel", &opts)`) to be eligible. The retained node’s `scroll_offset_y` is modified; clamping uses content height (from layout) and `scroll_max_h`.

### cui_inject_mouse_move

```c
void cui_inject_mouse_move(cui_ctx *ctx, int x, int y);
```

- **Semantics**: Sets the current pointer position in logical pixels. Used for hover hit-test and for determining which scroll container receives scroll. Call each frame (or when the platform reports mouse move) so that hover and scroll target are up to date.

### Hover query

```c
const char *cui_ctx_hovered_id(cui_ctx *ctx);
int         cui_ctx_is_hovered(cui_ctx *ctx, const char *id);
```

- **cui_ctx_hovered_id**: Returns the id of the widget currently under the pointer (after the last hover hit-test in end_frame), or NULL if none. Same depth rule as click: deepest (topmost) widget wins.
- **cui_ctx_is_hovered**: Returns 1 if `id` equals the current hovered widget id, 0 otherwise. Convenience for “if (cui_ctx_is_hovered(ctx, "btn")) { ... highlight ... }”.

### Scroll container with id

- **cui_scroll**: Extend to accept an optional id (e.g. `cui_scroll(ctx, "main", &opts)` or `cui_scroll_with_id(ctx, id, opts)`). If provided, the scroll node is eligible as a scroll target and can be looked up in the retained tree for offset updates.

## Existing API (unchanged)

- `cui_inject_click(ctx, x, y)`: Still used for click; click and hover both use the same hit-test pattern (layout then test). Hover uses (mouse_x, mouse_y); click uses (last_click_x, last_click_y).
- Platform `cursor_set`: Optional; app may call it when hovered_id changes. Not required for 0.4.0.

## Clamping

- **Scroll offset**: After applying scroll, `scroll_offset_y` is clamped to `[0, max(0, content_height - view_height)]` where `view_height = scroll_max_h` and `content_height` is the laid-out height of the scroll’s content (e.g. sum of children heights).
