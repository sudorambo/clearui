# Quickstart: Min/Max Constraint Clamping

## Build & Test

```bash
make clean && make all && make unit-tests && make integration-tests
```

All tests pass. No API changes.

## What changed

The layout engine in `src/layout/layout.c` now clamps each node’s `layout_w` and `layout_h` to the node’s `layout_opts.min_width`, `max_width`, `min_height`, `max_height` when those values are greater than 0. So:

- `cui_layout opts = { .max_width = 100 }; cui_column(ctx, &opts);` → the column’s width will be at most 100 logical pixels.
- `opts.min_height = 40` → the node’s height will be at least 40.
- If you don’t set min/max (0), behavior is unchanged.

## Verification

- Unit test: a node with `max_width = 100` and an intrinsic width larger than 100 ends up with `layout_w == 100`; a node with `min_height = 40` and intrinsic height 20 ends up with `layout_h == 40`.
- Existing layout and integration tests still pass.
