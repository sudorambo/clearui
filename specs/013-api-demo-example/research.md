# Research: Full API Demo Example

**Branch**: `013-api-demo-example` | **Date**: 2026-03-07

## R1: What API surface does the demo need to cover?

**Decision**: Cover every public function in `clearui.h` except `cui_run` (requires a real event loop) and `cui_dev_overlay` (requires `CUI_DEBUG` define, orthogonal to the demo purpose).

**Enumerated API** (from `include/clearui.h`):

| Category | Functions | Count |
|----------|-----------|-------|
| Lifecycle | `cui_create`, `cui_destroy`, `cui_running` | 3 |
| Frame | `cui_begin_frame`, `cui_end_frame` | 2 |
| Platform/RDI | `cui_set_platform`, `cui_set_rdi` | 2 |
| State | `cui_state`, `cui_frame_alloc`, `cui_frame_printf` | 3 |
| Style | `cui_push_style`, `cui_pop_style` | 2 |
| Widgets | `cui_button`, `cui_checkbox`, `cui_label`, `cui_label_styled`, `cui_icon_button`, `cui_text_input`, `cui_spacer` | 7 |
| Layout | `cui_center`, `cui_row`, `cui_column`, `cui_stack`, `cui_wrap`, `cui_scroll`, `cui_end` | 7 |
| Canvas | `cui_canvas`, `cui_draw_rect`, `cui_draw_circle`, `cui_draw_text` | 4 |
| A11y | `cui_aria_label`, `cui_tab_index` | 2 |
| Input | `cui_inject_click`, `cui_inject_key` | 2 |
| **Total** | | **34** |

**Rationale**: 34 functions is manageable in a single file under 200 lines. Each needs at least one call with a comment.

**Alternatives considered**: Splitting into multiple example files (one per category). Rejected — a single file is easier to discover and provides a complete picture.

## R2: Comment style for developer + AI readability

**Decision**: Use block comments at the top of each section (Platform, Widgets, Layout, etc.) explaining the pattern and *when you'd use it*. Inline comments only for non-obvious parameters (e.g., color format `0xAARRGGBB`).

**Rationale**: AI agents extract usage patterns from comments + code together. Block comments give enough context for few-shot prompting. Inline comments on every line create noise.

**Alternatives considered**: Full JSDoc-style per function call. Rejected — redundant with header docs and noisy.

## R3: Headless execution strategy

**Decision**: Use `cui_platform_stub_get()` and `cui_rdi_soft_get()` (already available). Run exactly 3 frames:
- Frame 1: Build full UI tree with all widgets and layout.
- Frame 2: Inject a click and key event, rebuild UI, observe state changes.
- Frame 3: Verify state propagation, print results.

**Rationale**: 3 frames is the minimum to demonstrate the inject→process→observe cycle, which is how ClearUI events work (inject in frame N, consumed in frame N+1).

**Alternatives considered**: Single frame. Rejected — can't demonstrate state persistence or input injection.

## R4: Output format

**Decision**: Print a section-by-section summary to stdout using `printf`:
```
=== ClearUI API Demo ===
Platform: stub (headless)
RDI: software
Frame 1: UI tree built (label, button, checkbox, text_input, ...)
Frame 2: Injected click on "+", Tab key
Frame 3: Counter = 1, checkbox toggled, focus moved
demo: PASS
```

**Rationale**: Human-readable, verifiable by CI, and serves as informal documentation of what the demo exercises.

## R5: Build integration

**Decision**: Add `demo` target to `Makefile`, following the same pattern as integration tests (link against `$(OBJS)`). Add `examples/` directory. Add link in README under a new "Examples" section.

**Rationale**: Consistent with existing build patterns. `examples/` is a standard directory name.

## R6: File structure

**Decision**: Place the file at `examples/demo.c`. This is the only new file (besides spec artifacts).

**Alternatives considered**: `tests/examples/demo.c` (rejected — examples are not tests), `src/examples/demo.c` (rejected — not library source).
