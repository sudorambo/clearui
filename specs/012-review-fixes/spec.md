# Feature Specification: Review Fixes

**Feature Branch**: `012-review-fixes`  
**Created**: 2026-03-07  
**Status**: Draft  
**Input**: Deep repository review findings — bugs, correctness issues, API design problems, test coverage gaps

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Correct Drawing Primitives (Priority: P1)

A developer calls `cui_draw_circle()` on a canvas and expects a circle to appear, not a rectangle.

**Why this priority**: This is a functional bug — the API promises a circle and delivers a square. It violates the Clarity principle (API readable as pseudocode) and will surprise every user of the canvas API.

**Independent Test**: Can be tested by calling `cui_draw_circle()` and verifying the emitted draw command is not `CUI_CMD_RECT`.

**Acceptance Scenarios**:

1. **Given** a canvas widget, **When** `cui_draw_circle(ctx, 100, 100, 50, color)` is called, **Then** a round draw command is emitted (rounded rect with r=50 or a new circle command).
2. **Given** `CUI_CMD_ROUNDED_RECT` exists in the enum, **When** a rounded rect push function is needed, **Then** `cui_draw_buf_push_rounded_rect()` is available and functional.

---

### User Story 2 - Safe Allocation Failure Handling (Priority: P1)

When arena allocation fails (e.g. OOM), the library must not silently corrupt internal state (parent stack, tree structure).

**Why this priority**: Pushing NULL onto the parent stack on alloc failure causes all subsequent child widgets to become orphans with no warning. This is a memory safety issue.

**Independent Test**: Force allocation failure and verify no crash or corruption in parent stack.

**Acceptance Scenarios**:

1. **Given** `cui_node_alloc` returns NULL, **When** `push_container` is called in `cui_center`/`cui_row`/`cui_column`/etc., **Then** NULL is NOT pushed onto the parent stack.
2. **Given** allocation failure in any widget function, **When** the frame continues, **Then** subsequent widgets attach to the correct parent (or are safely discarded).

---

### User Story 3 - Type-Safe Public API (Priority: P1)

A developer using `cui_set_platform()` and `cui_set_rdi()` should get a compile error if they pass the wrong type, not silent corruption.

**Why this priority**: `const void *` parameters discard all type safety at the most critical API boundary. This violates Clarity Over Cleverness.

**Independent Test**: Verify that passing a `cui_platform *` where `cui_rdi *` is expected produces a compiler warning/error.

**Acceptance Scenarios**:

1. **Given** `cui_set_platform()` signature, **When** the user passes `const cui_platform *`, **Then** compilation succeeds.
2. **Given** `cui_set_rdi()` signature, **When** the user passes a `const cui_platform *` by mistake, **Then** a compiler warning or error is produced.

---

### User Story 4 - Composable Accessibility State (Priority: P2)

A screen reader user navigating a focused, checked checkbox should hear both "checked" and "focused", not just one.

**Why this priority**: Accessibility is a core principle. Losing state information violates WCAG 2.1 AA and the Beautiful by Default & Accessibility First principle.

**Independent Test**: Build a tree with a focused checked checkbox and verify the a11y entry contains both states.

**Acceptance Scenarios**:

1. **Given** a checkbox that is both checked and focused, **When** `cui_a11y_build` runs, **Then** the entry's state includes both "checked" and "focused".

---

### User Story 5 - Fix `cui_text_input_opts` Type (Priority: P2)

A developer setting a placeholder on a text input should pass a string, not an integer.

**Why this priority**: The `placeholder` field is typed `int` but semantically represents a string. The widget implementation ignores `opts` entirely. This is a design debt that will confuse users.

**Independent Test**: Verify `cui_text_input_opts` compiles with a `const char *placeholder` field and the value is accessible in the widget.

**Acceptance Scenarios**:

1. **Given** `cui_text_input_opts`, **When** the user sets `.placeholder = "Enter name"`, **Then** it compiles and the widget stores the placeholder.

---

### User Story 6 - Hit-Test Text Inputs (Priority: P2)

Clicking on a text input should focus it, just like clicking a button.

**Why this priority**: Text inputs are focusable via Tab but not via click. This is inconsistent with the button/checkbox behavior.

**Independent Test**: Inject a click at a text input's layout position and verify focus is set.

**Acceptance Scenarios**:

1. **Given** a text input at position (100, 50), **When** `cui_inject_click(ctx, 100, 50)` is called, **Then** the text input receives focus.

---

### User Story 7 - Arena Alignment Validation (Priority: P2)

Passing a non-power-of-2 alignment to `cui_arena_alloc_aligned` should fail safely rather than produce undefined behavior.

**Why this priority**: Invalid alignment can cause the modulo math to produce wrong results. An assert or early return prevents silent UB.

**Independent Test**: Call `cui_arena_alloc_aligned` with alignment 0 and 3 (non-power-of-2), verify safe behavior (NULL return or assert).

**Acceptance Scenarios**:

1. **Given** `cui_arena_alloc_aligned(&a, 64, 0)`, **When** called, **Then** returns NULL.
2. **Given** `cui_arena_alloc_aligned(&a, 64, 3)`, **When** called, **Then** returns NULL.

---

### User Story 8 - Expanded Test Coverage (Priority: P3)

The test suite should cover the untested modules: a11y, render, frame_alloc, draw_cmd push functions, keyboard focus, style stack, and individual widget behavior.

**Why this priority**: Many core modules have no direct tests. Regressions in a11y, render, or focus management would go undetected.

**Independent Test**: Each new test file is independently runnable via `make`.

**Acceptance Scenarios**:

1. **Given** the test suite, **When** `make unit-tests` runs, **Then** tests for a11y, frame_alloc, draw_cmd, and focus/keyboard are included.
2. **Given** `test_arena` and `test_vault`, **When** they pass, **Then** they print "PASS" consistently with other tests.

---

### User Story 9 - Clean Up Dead Code and Inconsistencies (Priority: P3)

Remove or complete dead code (`CUI_CMD_ROUNDED_RECT` with no push function, `CUI_CMD_SCISSOR`, unused `CUI_DRAW_BUF_MAX`), standardize enum typedef style, and add a `libclearui.a` build target.

**Why this priority**: Dead code and inconsistencies create confusion for contributors.

**Independent Test**: Verify `CUI_CMD_ROUNDED_RECT` has a push function or is removed; verify `make libclearui.a` produces a static library.

**Acceptance Scenarios**:

1. **Given** the draw command API, **When** inspected, **Then** every `CUI_CMD_*` type has a corresponding push function.
2. **Given** the Makefile, **When** `make lib` is run, **Then** `libclearui.a` is produced.
3. **Given** all enum types, **When** inspected, **Then** they consistently use `typedef enum`.

---

### Edge Cases

- What happens when parent stack overflows (>16 levels of nesting)?
- How does the system handle zero-size allocation requests?
- What happens when the draw buffer is full during `cui_build_draw_from_tree`?
- How does the system handle a font file that doesn't exist?
- What happens when `cui_vault_get` is called with a key longer than 63 characters?

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: `cui_draw_circle` MUST emit a round draw command, not a rectangle
- **FR-002**: `cui_draw_buf_push_rounded_rect` MUST be implemented for `CUI_CMD_ROUNDED_RECT`
- **FR-003**: Widget allocation failure MUST NOT corrupt the parent stack or tree structure
- **FR-004**: `cui_set_platform` and `cui_set_rdi` MUST accept typed pointers, not `const void *`
- **FR-005**: `cui_a11y_build` MUST compose multiple states (checked + focused)
- **FR-006**: `cui_text_input_opts.placeholder` MUST be `const char *`
- **FR-007**: `hit_test_visit` MUST include `CUI_NODE_TEXT_INPUT`
- **FR-008**: `cui_arena_alloc_aligned` MUST validate alignment is a power of 2
- **FR-009**: Test suite MUST cover a11y, frame_alloc, draw_cmd, and keyboard focus
- **FR-010**: All enum types MUST use `typedef enum` consistently
- **FR-011**: Makefile MUST include a `libclearui.a` target
- **FR-012**: `test_arena` and `test_vault` MUST print "PASS" on success
- **FR-013**: Unused `CUI_DRAW_BUF_MAX` macro MUST be removed or repurposed

### Key Entities

- **cui_draw_cmd**: Draw command union — adding `push_rounded_rect`, possibly `push_scissor`
- **cui_node**: UI tree node — no structural changes, just behavioral fixes
- **cui_a11y_entry**: Accessibility entry — state field needs to support composition
- **cui_text_input_opts**: Public API struct — placeholder type change

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: `make all` compiles with zero warnings under `-Wall -Wextra -Wpedantic`
- **SC-002**: All existing tests continue to pass (no regressions)
- **SC-003**: New tests cover at least 5 previously untested modules
- **SC-004**: `cui_draw_circle` emits a non-rectangular draw command
- **SC-005**: Type mismatch in `cui_set_platform`/`cui_set_rdi` produces a compiler diagnostic
- **SC-006**: Focused + checked checkbox produces a combined a11y state string
