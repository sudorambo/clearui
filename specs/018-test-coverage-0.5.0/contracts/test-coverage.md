# Contract: Test Coverage (0.5.0)

Tests verify behavior of existing public APIs. No new public API is added. Tests may use internal headers (`context.h`, `node.h`, `draw_cmd.h`) to assert on draw buffer, layout, and style.

## Test contracts by area

### Canvas and draw commands

- **cui_canvas** + **cui_draw_rect** / **cui_draw_circle** / **cui_draw_text**
  - After a frame that opens a canvas, issues one or more draw calls, and closes with cui_end, the draw buffer (main and/or canvas buffer) MUST contain commands of the corresponding types (e.g. `CUI_CMD_RECT` or fill_rect, text) with coordinates and colors matching the arguments (within floating-point tolerance if applicable).
  - Test MAY use `cui_ctx_draw_buf(ctx)` and `cui_ctx_canvas_buf(ctx)` and inspect `buf->cmd[i].type` and `buf->cmd[i].u`.

### Label styled

- **cui_label_styled**
  - When a label is declared with `cui_label_styled(ctx, text, &style)`, the corresponding node in the declared tree after layout MUST have `text_color` (and optionally `text_decoration`) matching the provided style.
  - Test MAY use `cui_ctx_declared_root(ctx)` and walk to find the label node.

### Layout widgets

- **cui_spacer**
  - When `cui_spacer(ctx, w, h)` is used, the spacer node after layout MUST have `layout_w` and `layout_h` consistent with the declared w, h (or parent constraints).
- **cui_wrap**
  - When a wrap container has multiple children, after layout the children MUST be positioned in a flow (e.g. multiple rows or columns) such that wrapping semantics are observable (e.g. not all in one row if they would overflow).
- **cui_stack**
  - When a stack container has multiple children, after layout the children MUST overlap (e.g. same or contained layout rect) so that z-order is observable.

### Style stack

- **cui_push_style** / **cui_pop_style**
  - After push(style_A), push(style_B), pop(), the current style (e.g. `cui_ctx_current_style(ctx)`) MUST match style_A.
  - Nesting (push A, push B, pop, pop) MUST restore to the state before the first push.

### Frame allocator

- **cui_frame_alloc**
  - Allocations from `cui_frame_alloc(ctx, size)` in frame N MUST be valid until the end of frame N. After `cui_begin_frame(ctx)` for frame N+1, the allocator is reset; tests MUST NOT dereference the old pointer (use-after-free). Running the test under ASan MUST NOT report errors when the test only uses the allocation within the same frame.

### Hi-DPI scale buffer

- **scale_factor > 1**
  - When `cui_config.scale_factor` is set to a value > 1 (e.g. 2.f), after a frame with draw commands, the scaled buffer (e.g. `cui_ctx_scaled_buf(ctx)`) MUST contain coordinates scaled by that factor (e.g. logical (10,10) → physical (20,20) when scale_factor is 2).

### Edge cases

- **NULL ctx**
  - Public APIs that accept `cui_ctx *ctx` MUST tolerate `ctx == NULL` without crashing (typically early return). Tests MUST call each such API with NULL at least once where feasible.
- **Zero size / empty**
  - Zero-size spacer, zero capacity, or empty tree (no widgets between begin_frame and end_frame) MUST NOT cause crash or undefined behavior. Tests MAY assert no crash and, where defined, sensible default (e.g. empty draw buffer).

### Sanitizers

- **ASan / UBSan**
  - All new test binaries MUST be included in the targets run by `make unit-tests` (and `make integration-tests` if any integration tests are added). The existing `make asan` and `make ubsan` targets MUST run the full suite including new tests; no test MUST be excluded from sanitizer runs.
