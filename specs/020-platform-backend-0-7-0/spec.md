# Feature Specification: Platform Backend (0.7.0)

**Feature Branch**: `020-platform-backend-0-7-0`  
**Created**: 2026-03-07  
**Status**: Draft  
**Input**: ROADMAP Milestone 6 — At least one real platform adapter must ship for the library to be usable.

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Real window and one frame (Priority: P1)

A developer can build the library with an optional SDL3 (or one native) platform adapter, open a real window, run one frame (begin_frame, minimal UI, end_frame), and close the window. No headless stub required for this path.

**Why this priority**: Defines “usable” for the library; without a real window the library is test-only.

**Independent Test**: Integration test that links the SDL3 (or native) adapter, opens a window, calls cui_begin_frame / build minimal UI / cui_end_frame, then closes. Test passes if the window appears and closes without crash.

**Acceptance Scenarios**:

1. **Given** the repo built with the new platform adapter, **When** the integration test runs, **Then** a window is created, one frame is rendered, and the window is destroyed without crash.
2. **Given** an app using the adapter, **When** the app runs its main loop, **Then** events (mouse, keyboard, scroll, resize) are delivered and the app can drive ClearUI with them.

---

### User Story 2 — Platform capabilities (Priority: P2)

The adapter implements the full `cui_platform` contract: window create/destroy, event polling (mouse, keyboard, scroll, resize), clipboard get/set, cursor shape, and Hi-DPI scale factor so that apps get a complete desktop experience.

**Why this priority**: Required for real apps; clipboard and cursor are optional in the interface but expected when an adapter ships.

**Independent Test**: Manual or automated checks that clipboard_set/get work when supported, cursor_set changes cursor, and scale factor is reported (e.g. for cui_config.scale_factor).

**Acceptance Scenarios**:

1. **Given** the adapter, **When** the app sets clipboard text and another app pastes, **Then** the text matches (or adapter returns “not supported” cleanly).
2. **Given** the adapter, **When** the app calls cursor_set with a shape (e.g. text caret over input), **Then** the system cursor changes.
3. **Given** a Hi-DPI display, **When** the app queries scale factor from the adapter (or platform context), **Then** it can set cui_config.scale_factor so rendering is correct.

---

### User Story 3 — Bring your own platform (Priority: P2)

Documentation explains how to implement and plug in a custom `cui_platform` adapter so that developers can target a different windowing library or OS API without forking ClearUI.

**Why this priority**: Aligns with constitution’s “window-library agnostic” and supports native/X11/Wayland/Win32/Cocoa.

**Independent Test**: A reader can follow the doc and implement a minimal adapter (e.g. window_create, window_destroy, poll_events returning true) and wire it with cui_set_platform.

**Acceptance Scenarios**:

1. **Given** the “bring your own platform” docs, **When** a developer implements the required function pointers, **Then** they can use cui_set_platform(ctx, &my_platform, my_ctx) and run the loop.
2. **Given** an adapter with NULL clipboard/cursor/surface, **When** ClearUI uses the platform, **Then** it degrades gracefully (no crash; optional features simply unavailable).

---

### Edge Cases

- Window creation failure: adapter returns error from window_create; app must not call window_destroy with NULL or invalid ctx.
- Resize during frame: event polling should report new size; next frame uses new dimensions (or platform reports size on demand).
- Multiple windows: this milestone is single-window; multi-window is out of scope.
- CI without display: integration test that opens a real window may be skipped or run only when a display is available (e.g. optional or in a separate CI job).

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: An SDL3 platform adapter MUST be implemented in `cui_platform_sdl3.c` (or equivalent path) providing: window create/destroy, event polling (mouse, keyboard, scroll, resize), clipboard get/set, cursor shape, and Hi-DPI scale factor query where SDL3 supports it.
- **FR-002**: Alternatively, a native adapter for one OS (X11/Wayland, Win32, or Cocoa) MAY be implemented instead of or in addition to SDL3, with the same capabilities where that OS allows.
- **FR-003**: An integration test MUST open a real window, render one frame (cui_begin_frame, minimal UI, cui_end_frame), and close the window; the test MAY be skipped when no display is available (e.g. CI without GPU).
- **FR-004**: Documentation MUST describe how to implement and register a custom `cui_platform` adapter (required callbacks, optional ones, and cui_set_platform usage).

### Key Entities

- **cui_platform (existing)**: Function-pointer struct; window_create, window_destroy, window_get_size, poll_events; optional clipboard_get/set, cursor_set, surface_get/surface_destroy. Adapter implements this.
- **Platform context (cui_platform_ctx)**: Opaque per-window state; adapter allocates and passes to ClearUI.
- **Scale factor**: If not yet on the platform interface, either extend the platform (e.g. float (*scale_factor_get)(cui_platform_ctx *)) or document that the app sets cui_config.scale_factor from platform-specific APIs before creating the context.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: At least one built-in adapter (SDL3 or one native) allows opening a window and rendering one frame without crash.
- **SC-002**: Integration test for “real window, one frame, close” exists and passes when a display is available (or is skipped with a clear reason when not).
- **SC-003**: A developer can follow the docs to implement a minimal custom adapter and hook it up with cui_set_platform.
