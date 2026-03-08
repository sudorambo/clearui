# Data Model: Test Coverage (0.5.0)

**Branch**: `018-test-coverage-0.5.0` | **Date**: 2026-03-07

This milestone adds **no new runtime data structures**. It adds test programs that observe existing entities.

## Entities observed by tests

### Draw command buffer (existing)

- **Source**: `cui_draw_command_buffer` in `src/core/draw_cmd.h`; accessed via `cui_ctx_draw_buf(ctx)`, `cui_ctx_canvas_buf(ctx)`, `cui_ctx_scaled_buf(ctx)`.
- **Relevance**: Canvas/draw tests assert presence and fields of `CUI_CMD_RECT`, `CUI_CMD_TEXT`, etc. (e.g. `cmd[i].u.fill_rect.x,y,w,h,color`). Hi-DPI test asserts `cui_ctx_scaled_buf(ctx)` contains coordinates scaled by `scale_factor`.

### Declared tree (existing)

- **Source**: `cui_ctx_declared_root(ctx)` after `cui_end_frame`; nodes have `layout_w`, `layout_h`, `layout_x`, `layout_y`, `text_color`, etc.
- **Relevance**: Layout tests (spacer, wrap, stack) and style tests (label_styled) walk the declared tree and assert on node fields.

### Context style stack (existing)

- **Source**: `cui_ctx_current_style(ctx)`; style is applied to the next widget and stored on nodes.
- **Relevance**: Push/pop style tests assert that after pop, current style matches the previously pushed value.

### Frame allocator (existing)

- **Source**: Internal; no direct “reset” observable. Tests rely on two-frame sequence and ASan to ensure no use-after-free.
- **Relevance**: Frame allocator lifetime test does not add new state; it verifies that using frame_alloc in frame N and then beginning frame N+1 does not lead to sanitizer failure when the old pointer is not dereferenced.

## Test artifacts (not runtime)

- **Test binary**: Executable built from `tests/unit/test_*.c` (and optionally `tests/integration/*.c`), linked with ClearUI object files. Each test binary has a single `main()` that runs one or more test cases and exits 0 on success.
- **Makefile targets**: New phony targets or additions to `unit-tests` / `integration-tests` and `clean` so that `make unit-tests` and `make integration-tests` run the new tests; `make asan` and `make ubsan` (existing) run the same tests under sanitizers.

No new database, config files, or persistent state is introduced.
