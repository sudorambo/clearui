# Research: Scroll & Mouse (0.4.0)

Decisions made during planning. All NEEDS CLARIFICATION from the plan are resolved here.

## 1. Scroll target: which scroll node receives the wheel event

**Decision**: The scroll container **under the current mouse position** receives the scroll. To resolve the retained node for that scroll, we give scroll containers an optional **id** (e.g. `cui_scroll(ctx, id, opts)`) so we can look up the retained node by id and update `scroll_offset_y`. If no id is provided, scroll is not a target for inject_scroll (or we skip scroll injection when no scroll has id).

**Rationale**: Click and hover use widget id; scroll nodes today have no id. Adding an optional id keeps the pattern consistent and avoids matching by tree position. Alternative: match declared scroll under mouse to retained by walking the tree in parallel — doable but more fragile.

**Alternatives considered**: (1) Focus-based scroll target — rejected because scroll is usually tied to the widget under the pointer. (2) No id, match by position — possible but requires reliable declared/retained correspondence.

---

## 2. Content height for scroll clamping

**Decision**: Use the **layout height of the scroll container’s content** (sum or measured height of its children from the layout pass). Layout already runs in end_frame before we apply scroll. So: after layout, for a scroll node, content height = height of the scroll’s content (e.g. first child’s layout_h for a single column, or sum of children). Clamp `scroll_offset_y` to `[0, max(0, content_height - view_height)]` where `view_height = scroll_max_h`.

**Rationale**: Scroll offset must not go past content; layout already provides child dimensions. Existing code uses `scroll_max_h` as the visible viewport height.

**Alternatives considered**: Storing content height in the node — redundant if we can compute from layout each frame.

---

## 3. Pending scroll and application order

**Decision**: Context holds `pending_scroll_dx`, `pending_scroll_dy` (or just `pending_scroll_dy` for vertical). In `cui_end_frame`, after **layout** and after we know the scroll target (scroll node under mouse, by hit-test), we apply the pending scroll to that target’s **retained** node: add dy to `scroll_offset_y`, then clamp. Then clear pending scroll. If no scroll is under the mouse (or no scroll has an id), no-op and clear pending.

**Rationale**: Layout must run first so we have content height and hit-test positions. We update the retained node so that the next frame’s diff copies the new offset into the declared tree.

**Alternatives considered**: Applying scroll before layout — wrong because we need layout to clamp and to find the target.

---

## 4. Hover state storage and API

**Decision**: Context holds `mouse_x`, `mouse_y` (set by `cui_inject_mouse_move`) and `hovered_id[CUI_LAST_CLICKED_ID_MAX]` (set during a hover hit-test pass after layout, same depth rule as click: deepest widget wins). Expose via `const char *cui_ctx_hovered_id(cui_ctx *ctx)` and/or `int cui_ctx_is_hovered(cui_ctx *ctx, const char *id)`.

**Rationale**: One hovered widget per frame; app can style or highlight it. Matches existing `last_clicked_id` pattern.

**Alternatives considered**: Per-widget hover callbacks — more complex and not required for “highlight on hover”.

---

## 5. Cursor shape (optional)

**Decision**: **Document** that the app may call `ctx->platform->cursor_set(platform_ctx, shape)` when hovered_id changes (e.g. pointer over button). No new API in ClearUI for 0.4.0; optional implementation in demo or examples. Platform interface already has `cursor_set`.

**Rationale**: Keeps 0.4.0 scope focused on scroll and hover; cursor is a small addition once hover exists.

**Alternatives considered**: ClearUI automatically calling cursor_set when hovered widget has a “cursor” property — deferred to avoid new node fields and platform coupling in this milestone.
