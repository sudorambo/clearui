# Contract: cui_layout padding semantics (0.10.0)

## Type

`cui_layout` in `include/clearui.h`:

- `float padding` — shorthand for both axes.
- `float padding_x`, `float padding_y` — per-axis inner padding.

## Behavior

1. **Zero-init** `(cui_layout){}`: no padding (padding_x and padding_y effectively 0).
2. **Per-axis override**: For the X axis use `padding_x` if `padding_x > 0`, else `padding`. For the Y axis use `padding_y` if `padding_y > 0`, else `padding`.
3. **Shorthand**: If both `padding_x` and `padding_y` are 0, both axes use `padding`.

**Precedence**: `padding_x` / `padding_y` override `padding` when > 0. Document in header and README.

## Compatibility

No struct layout or default change; clarification only. Existing code using `padding` or `padding_x`/`padding_y` continues to work.
