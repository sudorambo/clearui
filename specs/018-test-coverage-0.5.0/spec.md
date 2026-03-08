# Feature Specification: Test Coverage (0.5.0)

**Feature Branch**: `018-test-coverage-0.5.0`  
**Created**: 2026-03-07  
**Status**: Draft  
**Input**: ROADMAP Milestone 4 — Fill gaps from readiness audit; every public API function exercised by at least one test.

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Canvas and draw commands (Priority: P1)

Maintainer or contributor runs tests that verify `cui_canvas`, `cui_draw_rect`, `cui_draw_circle`, and `cui_draw_text` produce expected draw buffer contents (commands, coordinates, colors).

**Why this priority**: Canvas is a core custom-draw API; untested draw paths are high risk for regressions.

**Independent Test**: Unit test builds a minimal context, declares a canvas with rect/circle/text draws, runs frame, inspects draw buffer (or RDI soft backend output) and asserts command presence and key fields.

**Acceptance Scenarios**:

1. **Given** a context with RDI and a frame, **When** app calls `cui_canvas` and `cui_draw_rect`/`cui_draw_circle`/`cui_draw_text`, **Then** draw buffer (or equivalent) contains corresponding commands with expected geometry/color.
2. **Given** canvas closed with `cui_end`, **When** frame completes, **Then** draw buffer reflects canvas content in correct order.

---

### User Story 2 — Styled label and layout widgets (Priority: P1)

Tests verify `cui_label_styled`, `cui_spacer`, `cui_wrap`, and `cui_stack` behavior: style application, layout sizing, flow wrapping, and z-order overlap.

**Why this priority**: Layout and styling are used by every UI; incorrect behavior is user-visible.

**Independent Test**: Unit tests that build a declared tree (or full context), run layout, and assert node dimensions, order, or style fields as specified.

**Acceptance Scenarios**:

1. **Given** a label with `cui_label_styled`, **When** frame is built, **Then** the label node (or rendered output) has the specified style (e.g. text_color).
2. **Given** `cui_spacer(ctx, w, h)`, **When** layout runs, **Then** spacer node has `layout_w`/`layout_h` matching (or constrained by) w, h.
3. **Given** a wrap container with multiple children, **When** layout runs, **Then** children are positioned in a flow (wrapping) rather than a single column/row.
4. **Given** a stack with overlapping children, **When** layout runs, **Then** children share the same layout rect (z-order overlap).

---

### User Story 3 — Style stack and frame allocator (Priority: P2)

Tests verify `cui_push_style` / `cui_pop_style` nesting and restoration, and `cui_frame_alloc` lifetime (freed after `cui_begin_frame`).

**Why this priority**: Style stack bugs cause wrong colors/fonts; frame allocator lifetime is critical for no-use-after-free.

**Independent Test**: Unit test pushes styles, declares widgets, pops, asserts current style restored; separate test allocates from frame, begins new frame, verifies previous allocation is no longer valid or that reset occurred.

**Acceptance Scenarios**:

1. **Given** push_style A, push_style B, pop_style, **When** query current style, **Then** it matches A.
2. **Given** allocation from `cui_frame_alloc` in frame N, **When** `cui_begin_frame` is called for frame N+1, **Then** frame allocator was reset (no double-free; allocator state reflects new frame).

---

### User Story 4 — Hi-DPI scale buffer path (Priority: P2)

Test verifies coordinate scaling when `scale_factor > 1` (scale_buf path).

**Why this priority**: Hi-DPI is required for production; unscaled or wrong-scaled coordinates cause blur or mislayout.

**Independent Test**: Set context `scale_factor > 1`, run frame with known draw commands, assert scaled buffer (or RDI submission) has coordinates scaled by scale_factor.

**Acceptance Scenarios**:

1. **Given** `config.scale_factor = 2.f`, **When** draw commands are generated and scaled buffer is built, **Then** coordinates in scaled buffer are scaled by 2.

---

### User Story 5 — Edge cases and sanitizers (Priority: P2)

Tests for NULL arguments, zero-size allocations, empty trees; CI or Makefile target for ASan/UBSan test runs.

**Why this priority**: Defensive behavior and no undefined behavior under sanitizers are required for stability.

**Independent Test**: Unit tests that call APIs with NULL ctx, zero capacity, empty tree; run existing test suite under ASan and UBSan (Makefile targets already exist from 0.2.0; ensure new tests are included).

**Acceptance Scenarios**:

1. **Given** NULL context passed to a public API, **When** call is made, **Then** no crash (early return or documented behavior).
2. **Given** zero-size buffer or empty tree, **When** layout/diff runs, **Then** no crash or OOB.
3. **Given** `make asan` / `make ubsan`, **When** all unit and integration tests run, **Then** no sanitizer errors.

---

### Edge Cases

- NULL `ctx` for each public API that accepts `cui_ctx *`.
- Zero `capacity` or zero-size layout (e.g. spacer 0,0).
- Empty declared tree (no widgets) through `cui_begin_frame` / `cui_end_frame`.
- Maximum nesting (e.g. CUI_PARENT_STACK_MAX) to ensure no overflow.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Every public API function listed in the ROADMAP (canvas/draw, label_styled, spacer, wrap, stack, push_style/pop_style, frame_alloc) MUST be exercised by at least one test.
- **FR-002**: Tests MUST verify draw buffer contents (or equivalent) for `cui_canvas` and `cui_draw_rect` / `cui_draw_circle` / `cui_draw_text`.
- **FR-003**: Tests MUST verify style application for `cui_label_styled` and style stack push/pop restoration.
- **FR-004**: Tests MUST verify layout sizing for `cui_spacer`, flow behavior for `cui_wrap`, and overlap behavior for `cui_stack`.
- **FR-005**: Tests MUST verify Hi-DPI scale_buf path when `scale_factor > 1`.
- **FR-006**: Tests MUST cover edge cases: NULL arguments, zero-size allocations, empty trees where applicable.
- **FR-007**: Makefile MUST include a target (or use existing `asan`/`ubsan`) so that all new tests run under ASan and UBSan; CI already runs sanitizers (0.2.0).

### Key Entities *(include if feature involves data)*

- **Test binary**: A small C program that links ClearUI, builds context (and optionally platform/RDI stubs), calls APIs under test, and asserts expected outcomes (draw buffer, layout fields, style, or no crash).
- **Draw command buffer**: Internal buffer inspected by tests to verify rect/circle/text commands and coordinates/colors.
- **Layout / style state**: Node layout_w, layout_h, layout_x, layout_y; current_style; verified after layout or after style push/pop.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Every public API listed in Milestone 4 (canvas, draw_*, label_styled, spacer, wrap, stack, push_style, pop_style, frame_alloc) is covered by at least one test that exercises it and asserts a defined outcome.
- **SC-002**: `make unit-tests` and `make integration-tests` pass; `make asan` and `make ubsan` pass with no new tests excluded.
- **SC-003**: Edge-case tests exist for NULL ctx, zero-size/empty inputs where relevant, and no sanitizer failures.
- **SC-004**: Hi-DPI scale_buf path is tested with scale_factor > 1 and coordinates verified.
