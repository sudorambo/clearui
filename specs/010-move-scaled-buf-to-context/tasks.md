# Tasks: Move the static scaled_buf in render.c to the context struct

**Input**: Design documents from `/specs/010-move-scaled-buf-to-context/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, quickstart.md

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[US1]**: User story 1 — scaled buffer owned by context, no static in render.c
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: Verify feature branch and baseline build.

- [X] T001 Verify feature branch `010-move-scaled-buf-to-context` and that `make clean && make all && make unit-tests && make integration-tests` passes from repo root before making changes.

---

## Phase 2: User Story 1 — Scaled buffer on context (R1, R2, R3)

**Goal**: The scaled draw command buffer is a member of `cui_ctx`, initialized and finalized with the main draw buffer; render uses it via a getter when scale != 1.f. No static draw buffer in render.c.

**Independent Test**: `grep -n 'static.*cui_draw_command_buffer\|static.*scaled_buf' src/core/render.c` returns no matches; when scale != 1.f the submit path uses a buffer from the context.

- [X] T002 [US1] In `src/core/context.c`: Add `cui_draw_command_buffer scaled_buf` to `struct cui_ctx` (e.g. after `canvas_cmd_buf`).

- [X] T003 [US1] In `src/core/context.c`: In `cui_create`, after calling `cui_draw_buf_init` for `draw_buf` and `canvas_cmd_buf`, call `cui_draw_buf_init(&ctx->scaled_buf, cap)` with the same `cap`. Include `scaled_buf` in the capacity check; on failure call `cui_draw_buf_fini(&ctx->scaled_buf)` (and the other buffers) before returning NULL.

- [X] T004 [US1] In `src/core/context.c`: In `cui_destroy`, call `cui_draw_buf_fini(&ctx->scaled_buf)` before any other teardown that uses the buffers (e.g. before `cui_draw_buf_fini(&ctx->draw_buf)` and `cui_draw_buf_fini(&ctx->canvas_cmd_buf)` or with them).

- [X] T005 [US1] In `src/core/context.c`: Implement `cui_draw_command_buffer *cui_ctx_scaled_buf(cui_ctx *ctx)` returning `ctx ? &ctx->scaled_buf : NULL`. In `src/core/context.h`: Declare `cui_draw_command_buffer *cui_ctx_scaled_buf(cui_ctx *ctx)`.

- [X] T006 [US1] In `src/core/render.c`: Remove any `static cui_draw_command_buffer scaled_buf` (or equivalent). In `cui_render_submit`, when `scale != 1.f`, obtain the destination buffer via `cui_ctx_scaled_buf(ctx)`, call `scale_buf(buf, scaled, scale)` with that buffer, and pass it to `rdi->submit(rdi_ctx, scaled)`.

---

## Phase 3: Polish & Verification

**Purpose**: Confirm no static in render and full build/test pass.

- [X] T007 Run `make clean && make all && make unit-tests && make integration-tests` and confirm zero warnings and all tests PASS. Run `grep -n 'static.*cui_draw_command_buffer\|static.*scaled_buf' src/core/render.c` and confirm no matches.

---

## Dependencies & Execution Order

- **Phase 1**: T001 first.
- **Phase 2**: T002 before T003, T004, T005 (struct member must exist). T003 and T004 and T005 all touch context.c; do T002 → T003 → T004 → T005. T006 (render.c) depends on T005 (getter); do T006 after T005.
- **Phase 3**: T007 after all implementation tasks.

### Suggested order

1. T001  
2. T002 → T003 → T004 → T005  
3. T006  
4. T007  

---

## Parallel Opportunities

- None for this feature; context.c and render.c are modified in sequence (getter must exist before render uses it).

---

## Implementation Strategy

### MVP (User Story 1)

1. T001 (baseline).  
2. T002–T005 (context: struct, create, destroy, getter).  
3. T006 (render: use getter, no static).  
4. T007 (verify).  

### Acceptance (from spec)

- No `static` draw command buffer in `src/core/render.c`.  
- When scale != 1.f, scaled output is written to a buffer owned by the context.  
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
