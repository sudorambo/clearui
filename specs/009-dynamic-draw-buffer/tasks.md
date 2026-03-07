# Tasks: Dynamic Draw Command Buffer / Configurable Capacity

**Input**: Design documents from `/specs/009-dynamic-draw-buffer/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[US1]**: User story 1 — configurable draw buffer capacity at create time
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: No new project structure; verify baseline.

- [X] T001 Verify feature branch `009-dynamic-draw-buffer` and that `make clean && make all && make unit-tests && make integration-tests` passes from repo root before making changes.

---

## Phase 2: Foundational (Buffer Representation)

**Purpose**: Change buffer from fixed array to pointer + capacity so context can own heap-allocated buffers. Required before wiring config and context.

- [X] T002 In `src/core/draw_cmd.h`: Change `cui_draw_command_buffer` to hold `cui_draw_cmd *cmd`, `size_t capacity`, `size_t count`. Retain `#define CUI_DRAW_BUF_MAX 1024` as the default capacity constant (used when capacity is unspecified). Remove the fixed array member.

- [X] T003 In `src/core/draw_cmd.c`: Add `void cui_draw_buf_init(cui_draw_command_buffer *buf, size_t capacity)` (allocate `cmd` with `capacity` elements; set `count = 0`) and `void cui_draw_buf_fini(cui_draw_command_buffer *buf)` (free `cmd`, set to NULL, capacity/count to 0). Update `cui_draw_buf_clear` to set `count = 0` only. Update `cui_draw_buf_push_rect`, `cui_draw_buf_push_line`, `cui_draw_buf_push_text` to check `buf->count < buf->capacity` instead of `buf->count >= CUI_DRAW_BUF_MAX`; return -1 when full. Declare init/fini in `draw_cmd.h`.

**Checkpoint**: Buffer type has capacity and heap-backed `cmd`; init/fini and push/clear use them. Context and render not yet updated.

---

## Phase 3: User Story 1 — Configurable Capacity (R1, R2, R3)

**Goal**: Application can set draw buffer capacity at create time via `cui_config`; default 1024; overflow returns -1.

**Independent Test**: Create context with `config.draw_buf_capacity = 2048`, run one frame, assert buffer capacity is 2048 (e.g. via optional unit test or manual check). Create with `draw_buf_capacity = 0` or unset, assert default 1024.

- [X] T004 [P] [US1] In `include/clearui.h`: Add `size_t draw_buf_capacity` to `cui_config`. Document: 0 = use default 1024; > 0 = max draw commands per buffer. No other API changes.

- [X] T005 [US1] In `src/core/context.c`: In `cui_create`, after copying config, compute `cap = config->draw_buf_capacity > 0 ? config->draw_buf_capacity : 1024`. Call `cui_draw_buf_init(&ctx->draw_buf, cap)` and `cui_draw_buf_init(&ctx->canvas_cmd_buf, cap)`. In `cui_destroy`, call `cui_draw_buf_fini(&ctx->draw_buf)` and `cui_draw_buf_fini(&ctx->canvas_cmd_buf)` before any other teardown that uses the buffers. Ensure create returns NULL on init failure if needed.

- [X] T006 [US1] In `src/core/render.c`: In `scale_buf`, replace the loop guard `dst->count < CUI_DRAW_BUF_MAX` with `dst->count < dst->capacity`. Ensure the destination buffer has at least the same capacity as the source (e.g. when scale != 1.f, use a context-owned scaled buffer of the same capacity as the main draw buffer, or a temporary buffer of size `src->capacity`; init/fini for a context-owned scaled buffer in create/destroy if chosen). Update any other use of `CUI_DRAW_BUF_MAX` in this file to use buffer `capacity`/`count`.

**Checkpoint**: Configurable capacity wired; create/destroy own buffer lifecycle; render path uses capacity/count. All existing tests should pass.

---

## Phase 4: Polish & Verification (R4)

**Purpose**: Optional unit test and full build/test pass.

- [X] T007 [P] [US1] (Optional) In `tests/unit/test_draw_buf.c` (or existing test file): Add a test that creates a context with `draw_buf_capacity = 2048`, runs layout/build once, and asserts the draw buffer has capacity 2048 (e.g. via a getter if added, or by pushing 1025 commands and verifying no truncation). Add a test that with default config (0), capacity is 1024. Wire the test into the Makefile if a new file is created.

- [X] T008 Run `make clean && make all && make unit-tests && make integration-tests` and confirm zero warnings and all tests PASS.

---

## Dependencies & Execution Order

- **Phase 1**: T001 first (baseline).
- **Phase 2**: T002 before T003 (struct change before init/fini and push logic).
- **Phase 3**: T004 can run in parallel with T002/T003. T005 depends on T002, T003, T004. T006 depends on T002 (buffer has capacity/count); can run after T003, and should run after T005 if render uses a context-owned scaled buffer.
- **Phase 4**: T007 optional; T008 after all implementation tasks.

### Suggested order

1. T001  
2. T002 → T003  
3. T004 (parallel with T002/T003 if desired), then T005, then T006  
4. T007 (optional), T008  

---

## Parallel Opportunities

- T004 (config field) is independent of T002/T003 and can be done in parallel with Phase 2.
- T007 (optional test) can be written in parallel with T005/T006 if test only checks config and capacity after create.

---

## Implementation Strategy

### MVP (User Story 1)

1. T001 (baseline).  
2. T002, T003 (buffer representation + init/fini).  
3. T004, T005, T006 (config + context + render).  
4. T008 (full verify).  

### Acceptance (from spec)

- Application can set draw buffer capacity at create time (e.g. 2048) and get that capacity.  
- Default capacity with no config change is 1024.  
- When capacity is exceeded and growth is not enabled, push returns -1.  
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
