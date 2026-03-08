# Research: Test Coverage (0.5.0)

**Branch**: `018-test-coverage-0.5.0` | **Date**: 2026-03-07

## Decisions

### How to assert on draw buffer contents

**Decision**: Tests that need to verify canvas/draw commands use the internal API: include `src/core/context.h` and `src/core/draw_cmd.h`. After `cui_end_frame`, call `cui_ctx_draw_buf(ctx)` or `cui_ctx_canvas_buf(ctx)` when inside canvas; inspect `buf->cmd[i].type` and `buf->cmd[i].u` (e.g. `fill_rect`, `text`) and assert on coordinates and colors. Tests are linked with the full library and thus have access to internal headers; this is the same pattern as `test_focus.c` (which uses `cui_ctx_focused_id` and context setup).

**Rationale**: The public API does not expose draw buffer contents; test-only use of context accessors is acceptable and already used (e.g. `cui_ctx_retained_root` in test_scroll.c). No new public API is added.

**Alternatives considered**: (1) Expose a “test mode” in the public API to return draw commands — rejected to avoid polluting public API. (2) Use RDI soft backend callbacks to capture commands — possible but more complex; direct buffer inspection is simpler and sufficient.

---

### How to assert on layout (spacer, wrap, stack)

**Decision**: After `cui_end_frame`, use `cui_ctx_declared_root(ctx)` to get the declared tree (which has been laid out). Walk the tree and read `node->layout_w`, `node->layout_h`, `node->layout_x`, `node->layout_y`. For spacer, assert that the spacer node has the expected dimensions (or are constrained by parent). For wrap, assert children have positions that indicate wrapping (e.g. multiple rows). For stack, assert overlapping children share the same (or contained) layout rect. Tests include `src/core/context.h` and `src/core/node.h`.

**Rationale**: Layout results are stored on nodes; declared root is available after end_frame. No new API required.

**Alternatives considered**: Assert only on presence of node types — rejected as too weak; we need to assert layout semantics.

---

### How to assert on style (label_styled, push_style/pop_style)

**Decision**: For `cui_label_styled`, after building a frame that includes a styled label, find the label node in the declared tree and assert `node->text_color` (and optionally `text_decoration`) matches the style passed. For push/pop, call `cui_ctx_current_style(ctx)` after push/pop sequence and assert fields (e.g. `cui_ctx_current_style(ctx)->text_color`). Style is applied at declaration time and stored on the node; current_style is part of context and used for the “next” widget.

**Rationale**: Existing accessors and node fields are sufficient; no new API.

---

### Frame allocator lifetime test

**Decision**: Allocate memory with `cui_frame_alloc(ctx, n)` in frame N, record the pointer. Call `cui_begin_frame(ctx)` for frame N+1. Option A: Allocate again with the same size and assert the new pointer may equal the previous one (bump reset). Option B: Do not dereference the old pointer after begin_frame (use-after-free would be caught by ASan). Option C: Document that frame allocator reset is internal; test that two frames each calling frame_alloc yield valid pointers and that running under ASan does not report use-after-free when the first frame’s allocation is not used after the second begin_frame. Prefer Option C plus ASan: no need to expose allocator internals; the test simply runs a two-frame sequence with frame_alloc in the first and verifies no crash and no ASan error.

**Rationale**: Frame allocator API does not expose “has been reset”; relying on ASan and correct usage is the minimal and safe approach.

**Alternatives considered**: Adding a test-only “frame allocator reset count” — rejected; keep tests depending on existing behavior and sanitizers.

---

### Hi-DPI scale_buf test

**Decision**: Create context with `cui_config.scale_factor = 2.f`. Build a frame that produces at least one draw command with known coordinates (e.g. a button or a canvas rect at (10, 10)). After `cui_end_frame`, the main draw buffer holds logical coordinates; the scaled buffer is filled by the render path. Inspect `cui_ctx_scaled_buf(ctx)` (or the path that fills it) and assert that coordinates in the scaled buffer are scaled (e.g. 20, 20 for logical 10, 10). If the scaled buffer is only filled at submit time, the test may need to call the internal function that copies/scales from draw_buf to scaled_buf and then inspect scaled_buf. Research render.c to see where scale happens.

**Rationale**: Scale factor is part of the public config; tests must verify that scaling is applied. Internal access to scaled_buf is already available via `cui_ctx_scaled_buf(ctx)`.

**Alternatives considered**: Testing only via RDI submit — possible but would require RDI to expose submitted data; internal buffer inspection is simpler.

---

### Edge cases and sanitizers

**Decision**: (1) NULL ctx: for each public API that takes `cui_ctx *ctx`, add a test that calls the API with `ctx == NULL` and expects no crash (early return). (2) Zero-size / empty: call with zero capacity, zero spacer size, or empty tree (begin_frame, end_frame with no widgets) and assert no crash. (3) ASan/UBSan: Makefile already has `asan` and `ubsan` targets (0.2.0); ensure all new test binaries are built and run by those targets (they use the same `unit-tests` and `integration-tests` phony targets), so no Makefile change is strictly required unless a test is excluded. Verify that new tests are included in the list of binaries run by `make unit-tests` and that `make asan`/`make ubsan` run them.

**Rationale**: Edge-case tests improve robustness; sanitizers already in CI; we only need to wire new tests into existing targets.

---

### Test file organization

**Decision**: One test file per theme (test_canvas_draw.c, test_label_styled.c, test_spacer.c, test_wrap.c, test_stack.c, test_style_stack.c, test_frame_alloc.c, test_scale_buf.c, test_edge_cases.c) or group related tests (e.g. test_layout_widgets.c for spacer, wrap, stack) to avoid a proliferation of tiny binaries. Prefer one binary per logical area to keep Makefile and test names clear; multiple test cases (assertions) can live in one main() or in separate functions called from main().

**Rationale**: Matches existing pattern (test_focus.c, test_text_input.c, test_scroll.c); easier to run a single test during development.
