# Feature Spec: Move the static scaled_buf in render.c to the context struct

**Branch**: `010-move-scaled-buf-to-context` | **Date**: 2026-03-07

## Problem

When Hi-DPI scaling is used (`scale != 1.f`), the render path needs a temporary buffer to hold scaled draw commands. If that buffer is a **file-static** variable in `render.c` (e.g. `static cui_draw_command_buffer scaled_buf`), it introduces implicit global state: lifetime and capacity are hidden, and the buffer is not tied to the context that owns the pipeline. The Constitution (Clarity, Explicit Ownership) favors context-owned state over static storage.

## Goals

1. **Eliminate static scaled buffer**: Remove any `static cui_draw_command_buffer scaled_buf` (or equivalent) from `render.c`.
2. **Context-owned scaled buffer**: Store the scaled output buffer in the context struct (`cui_ctx`), with the same lifecycle as the main draw buffer (initialized in `cui_create`, freed in `cui_destroy`).
3. **Render uses context**: In `cui_render_submit`, when scale != 1.f, use the context’s scaled buffer (e.g. via a getter like `cui_ctx_scaled_buf(ctx)`) instead of a static buffer.

## Requirements

1. **R1**: The scaled draw command buffer used for Hi-DPI output MUST be a member of `struct cui_ctx` (or otherwise owned by the context), not a file-static in `render.c`.
2. **R2**: The scaled buffer MUST be initialized and finalized with the same capacity as the main draw buffer (so scaling never truncates). Initialization in `cui_create`, finalization in `cui_destroy`.
3. **R3**: No new public API required beyond an internal getter if needed (e.g. `cui_ctx_scaled_buf`). Existing tests MUST continue to pass.

## Scope

- **In scope**: `src/core/render.c` (remove static, use context buffer), `src/core/context.c` (add `scaled_buf` to struct, init/fini, getter), `src/core/context.h` (declare getter if added).
- **Out of scope**: Changing RDI contract, draw command format, or public `clearui.h` API.

## Acceptance Criteria

- There is no `static` draw command buffer in `render.c`.
- When scale != 1.f, the scaled output is written to a buffer owned by the context.
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
