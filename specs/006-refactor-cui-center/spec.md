# Feature Spec: Refactor `cui_center` to use `push_container`

**Branch**: `006-refactor-cui-center` | **Date**: 2026-03-07

## Problem

`cui_center` in `src/widget/layout.c` (lines 39–54) manually duplicates the node-allocation, root/parent attachment, and parent-push logic that `push_container` (lines 21–37) already encapsulates. Every other container function (`cui_row`, `cui_column`, `cui_stack`, `cui_wrap`) delegates to `push_container`; `cui_center` is the sole outlier.

This duplication means:
- Bug fixes to container setup (e.g. the `last_child` change in 005) must be applied in two places.
- The pattern is inconsistent, making the code harder to read.
- Future container features (e.g. layout opts for `cui_center`) require touching duplicated code.

## Requirements

1. **R1**: Replace the body of `cui_center` with a single call to `push_container(ctx, CUI_NODE_CENTER, NULL, 400, 300)`.
2. **R2**: The public API signature `void cui_center(cui_ctx *ctx)` MUST NOT change.
3. **R3**: All existing tests MUST continue to pass — behavior is identical.
4. **R4**: No other files need modification.

## Scope

- **In scope**: `src/widget/layout.c` — body of `cui_center` only.
- **Out of scope**: Adding `cui_layout *opts` parameter to `cui_center`, changing default dimensions, modifying `push_container`.

## Acceptance Criteria

- `cui_center` body is exactly: `if (!ctx) return; push_container(ctx, CUI_NODE_CENTER, NULL, 400, 300);`
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
- `grep -c 'cui_ctx_root_ptr\|cui_node_alloc\|cui_ctx_push_parent' src/widget/layout.c` shows these functions called only inside `push_container`, not duplicated in `cui_center`.
