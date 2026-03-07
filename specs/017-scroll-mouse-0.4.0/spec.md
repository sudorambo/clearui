# Feature Specification: Scroll & Mouse (0.4.0)

**Feature Branch**: `017-scroll-mouse-0.4.0`  
**Created**: 2026-03-07  
**Status**: Draft  
**Input**: ROADMAP.md Milestone 3 — scroll containers and mouse hover.

## User Scenarios & Testing

### User Story 1 - App injects scroll wheel to move scroll content (Priority: P1)

As an application I can call `cui_inject_scroll(ctx, dx, dy)` so that scroll wheel events from the platform update the scroll offset of the scroll container under the pointer (or focused).

**Why this priority**: Scroll containers exist but have no input; this completes basic scroll UX.

**Independent Test**: Declare a scroll container with content taller than viewport; inject scroll; assert scroll offset is updated and clamped to content bounds.

**Acceptance Scenarios**:

1. **Given** a scroll container with content height > max_height, **When** the app injects scroll (e.g. dy &lt; 0 for scroll down), **Then** the scroll offset increases (content moves up) and is clamped so the visible region stays within content.
2. **Given** scroll at max offset, **When** the app injects more scroll in the same direction, **Then** offset does not go below 0 or above (content_height - view_height).

---

### User Story 2 - Mouse hover state is tracked and exposed (Priority: P1)

As an application I can call `cui_inject_mouse_move(ctx, x, y)` so that the library knows the pointer position, and I can query whether a widget (e.g. button) is hovered for highlight.

**Why this priority**: Hover feedback is expected for buttons and interactive elements.

**Independent Test**: Declare a button; inject mouse move to a position inside the button; assert that the widget is reported as hovered (e.g. `cui_ctx_hovered_id` or `cui_ctx_is_hovered(ctx, id)`).

**Acceptance Scenarios**:

1. **Given** a button with id "btn", **When** mouse is moved to (x,y) inside the button's layout rect, **Then** the app can detect hover (e.g. `cui_ctx_hovered_id(ctx)` equals "btn" or `cui_ctx_consume_hover(ctx, "btn")` returns 1).
2. **Given** mouse moved outside the button, **Then** that widget is no longer hovered (last-hit wins for overlapping).

---

### User Story 3 - Optional cursor shape (Priority: P2)

As an application I can request a cursor shape change (e.g. pointer over buttons) via the platform; the platform interface already has `cursor_set`. This milestone may only document usage; implementation can be optional.

**Independent Test**: If implemented, set cursor over a button and assert platform `cursor_set` was called with the expected shape.

---

### User Story 4 - Tests for scroll and hover (Priority: P2)

As a maintainer I have unit tests for scroll offset clamping and an integration test that scrolls a container and verifies the visible content region (or offset).

**Independent Test**: Run `make unit-tests` and `make integration-tests`; new tests pass.

---

### Edge Cases

- **Scroll**: No scroll container under pointer — either no-op or use focused/previous target; document behavior. Multiple nested scrolls: scroll the deepest under pointer.
- **Scroll bounds**: Content height may be from layout (sum of children); clamp offset to [0, max(0, content_h - view_h)].
- **Hover**: Hit-test after layout; same depth rule as click (deepest wins). Hover id cleared or updated each frame.

## Requirements

### Functional Requirements

- **FR-001**: The library MUST provide `cui_inject_scroll(ctx, dx, dy)` (or equivalent) for scroll wheel events. Values are in logical pixels (e.g. dy &lt; 0 = scroll down = content up).
- **FR-002**: Scroll wheel MUST update the `scroll_offset_y` of the scroll container that is the target (e.g. under the mouse, or focused scroll). Offset MUST be clamped to [0, max(0, content_height - view_height)].
- **FR-003**: The library MUST provide `cui_inject_mouse_move(ctx, x, y)` so that hover state can be computed after layout/hit-test.
- **FR-004**: The library MUST expose hover state to the app (e.g. current hovered widget id, or per-widget query) so that widgets can highlight on hover.
- **FR-005**: Optional: cursor shape changes via `cui_platform.cursor_set`; document or implement for common cases (e.g. pointer over button).
- **FR-006**: Unit tests MUST cover scroll offset clamping (inject scroll, assert offset in range).
- **FR-007**: An integration test MUST scroll a container and verify visible content region or offset.

### Key Entities

- **Scroll container**: Existing `CUI_NODE_SCROLL` with `scroll_offset_y`, `scroll_max_h`; content height from layout (children height).
- **Scroll event**: Pending (dx, dy) in context; consumed in end_frame and applied to the scroll node under mouse (or chosen target).
- **Hover state**: Current mouse (x, y); after layout, hit-test determines which widget is hovered; store hovered widget id (e.g. `last_hovered_id` or `hovered_id`).

## Success Criteria

- **SC-001**: `cui_inject_scroll` updates scroll offset of the target scroll container; offset is clamped.
- **SC-002**: `cui_inject_mouse_move` and hover query allow buttons (or widgets) to show hover highlight.
- **SC-003**: Optional cursor shape is documented or implemented.
- **SC-004**: New unit test(s) for scroll clamping and integration test for scroll + visible region pass in CI.
