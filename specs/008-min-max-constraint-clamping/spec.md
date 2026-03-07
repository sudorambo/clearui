# Feature Spec: Implement Min/Max Constraint Clamping in the Layout Engine

**Branch**: `008-min-max-constraint-clamping` | **Date**: 2026-03-07

## Problem

`cui_layout` and `cui_node_layout` already define `max_width`, `min_width`, `max_height`, `min_height` (see `include/clearui.h`, `src/core/node.h`). These are copied from the public API into nodes via `copy_layout_opts` in `src/widget/layout.c`, but the layout engine in `src/layout/layout.c` never uses them. Computed `layout_w` and `layout_h` are written without clamping, so developers cannot constrain node size (e.g. "this column at most 300px wide" or "this row at least 40px tall").

## Goals

1. **Clamp after size computation**: After the layout engine assigns `layout_w` and `layout_h` to a node (in measure and/or in the position pass), apply min/max constraints from `n->layout_opts` so that the final size lies within `[min_width, max_width]` and `[min_height, max_height]` when those constraints are set.
2. **Convention for “no constraint”**: Use a single convention so existing code and new opts behave predictably. Treat **0 as “no constraint”** (do not clamp to 0): only when `min_width > 0` do we enforce a minimum width; only when `max_width > 0` do we enforce a maximum width (and similarly for height).
3. **No API change**: The public `cui_layout` struct and container APIs remain unchanged; only the layout engine implementation is updated to respect existing fields.

## Requirements

1. **R1**: In `src/layout/layout.c`, after every place that sets `n->layout_w` or `n->layout_h` (measure pass and run_layout pass), clamp to `n->layout_opts.min_width` / `max_width` and `min_height` / `max_height` when the corresponding option is > 0.
2. **R2**: Clamping is inclusive: final `layout_w` in `[min_width, max_width]` when both are > 0; if only one bound is set, clamp only to that bound. Same for height.
3. **R3**: All existing tests pass. Add at least one unit test that sets `min_width`/`max_width` (or min/max height) on a container or leaf and asserts the node’s final `layout_w`/`layout_h` is within the specified range.

## Scope

- **In scope**: Layout engine only (`src/layout/layout.c`); optional unit test in `tests/unit/test_layout.c`.
- **Out of scope**: Changing `cui_layout` or `cui_node_layout` structs, new public API, flex distribution based on min/max (future improvement).

## Acceptance Criteria

- A node with `layout_opts.max_width = 100` and computed width 150 has final `layout_w == 100`.
- A node with `layout_opts.min_height = 40` and computed height 20 has final `layout_h == 40`.
- When `min_*` or `max_*` is 0, no clamping is applied for that bound.
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
