# Data Model: Min/Max Constraint Clamping

## Existing entity: `cui_node_layout` (unchanged)

**Location**: `src/core/node.h` (and public `cui_layout` in `include/clearui.h`)

Relevant fields:

| Field       | Type  | Meaning (after this feature) |
|------------|-------|-------------------------------|
| `min_width`  | float | If > 0: layout_w must be ≥ this. |
| `max_width`  | float | If > 0: layout_w must be ≤ this. |
| `min_height` | float | If > 0: layout_h must be ≥ this. |
| `max_height` | float | If > 0: layout_h must be ≤ this. |

**Convention**: 0 means “no constraint” for that bound. So we only clamp when the corresponding option is > 0.

## Clamp semantics

- **Width**: If `min_width > 0`, then `layout_w = max(layout_w, min_width)`. If `max_width > 0`, then `layout_w = min(layout_w, max_width)`. If both are set, apply both (order: e.g. clamp to min then to max, or vice versa; result is in `[min_width, max_width]`).
- **Height**: Same for `layout_h`, `min_height`, `max_height`.
- If only one bound is set, the other is effectively unbounded (we only apply the set bound).

## No new state

No new structs or fields. The layout engine simply applies the above rules after assigning `layout_w` and `layout_h` so that the stored values satisfy the constraints.
