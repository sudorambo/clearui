# Feature Spec: Start the Diff Engine

**Branch**: `011-diff-engine` | **Date**: 2026-03-07

## Problem

ClearUI uses a declarative immediate-style API: each frame the app builds a "declared" UI tree (in the frame arena). The library currently has a diff entry point `cui_diff_run(declared, &retained)` in `src/core/diff.c`, but it is a stub that just sets `*retained = declared`. So there is no real reconciliation: no persistence of focus, scroll position, text input content, or other state across frames. This is the library's differentiating feature over pure immediate-mode—a retained tree that mirrors the declared structure and carries state between frames, so that layout and draw can run on a stable tree with correct state.

## Goals

1. **Structural reconciliation**: The diff engine MUST produce a retained tree that mirrors the declared tree structure (same types, same child order). Matching of declared nodes to retained nodes SHOULD use a stable key where available (e.g. `button_id` for buttons) or child index, so that state can be preserved for the "same" logical widget.
2. **State transfer**: For nodes that match, copy persistent state from retained into declared (or into the updated retained node) so that layout and downstream code (draw, a11y) see the correct state—e.g. `scroll_offset_y`, `text_input_buf`, focus ownership.
3. **Lifecycle**: Retained nodes MUST live in a persistent allocator (e.g. heap or a non-reset arena). Orphaned retained nodes (removed from declared structure) MUST be freed. No `free()` on declared nodes (arena reset handles them).
4. **Minimal API**: Keep `cui_diff_run(cui_node *declared, cui_node **retained)` as the single entry point; no new public API required. Context continues to own `retained_root` and pass it to the diff.

## Requirements

1. **R1**: After `cui_diff_run(declared, &retained)`, the retained tree MUST have the same structure as the declared tree (root type, child count and order, node types). Retained nodes MAY be reused when the declared structure matches the previous retained structure.
2. **R2**: Persistent per-widget state (e.g. scroll offset, text input buffer, checkbox checked pointer) MUST be preserved for matched nodes and made visible to layout/draw (e.g. by copying from retained into declared before layout, or by running layout on the retained tree after updating it).
3. **R3**: Retained tree storage and allocation MUST respect Constitution: explicit ownership, no hidden allocs. Document where retained nodes are allocated and who frees them (e.g. context owns a retained arena or heap pool; diff allocates/frees as needed).
3. **R4**: All existing tests MUST pass. No new mandatory public API; internal helpers in `diff.c` are fine.

## Scope

- **In scope**: `src/core/diff.c` (reconcile algorithm), `src/core/diff.h` (no signature change), `src/core/context.c` (retained_root lifecycle: where it is stored, how it is passed to diff). Optionally `src/core/node.h` if a key/identity field is needed for matching.
- **Out of scope**: Changing the public ClearUI widget API; changing layout/draw function signatures; full "virtual DOM" diff with patches (future improvement).

## Acceptance Criteria

- Running the same UI declaration two frames in a row, the retained tree after the second frame has the same structure as the declared tree and preserves state (e.g. text input content or scroll position) where the widget is matched.
- Removing a widget from the declaration causes the corresponding retained subtree to be freed (no leak).
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
