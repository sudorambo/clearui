# Feature Specification: Theming (0.6.0)

**Feature Branch**: `019-theming-0.6.0`  
**Created**: 2026-03-07  
**Status**: Draft  
**Input**: ROADMAP Milestone 5 — Replace hardcoded `theme.h` #defines with runtime theming for a 1.0-ready library.

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Set theme at runtime (Priority: P1)

App sets a theme (colors, radii, font size, focus ring) via `cui_set_theme(ctx, &theme)` so that all subsequent draw commands and layout use that theme until changed. No theme set → built-in default (current light look).

**Why this priority**: Core requirement; without it theming is compile-time only.

**Independent Test**: Create ctx, call `cui_set_theme(ctx, &custom_theme)` with distinct colors, build a frame with a button, after `cui_end_frame` inspect draw buffer and assert button background color matches custom theme.

**Acceptance Scenarios**:

1. **Given** a context, **When** `cui_set_theme(ctx, &theme)` is called with non-default colors, **Then** the next frame’s draw commands use those colors (e.g. button bg, focus ring).
2. **Given** a context with no theme set, **When** a frame is built, **Then** draw commands use the built-in default theme (same as current behavior).

---

### User Story 2 — Dark theme preset (Priority: P2)

Library ships a dark theme preset (e.g. dark backgrounds, light text) that the app can apply with one call so users can switch to dark mode without defining every field.

**Why this priority**: Common need; demonstrates preset pattern and improves UX.

**Independent Test**: Apply dark preset, build frame, assert draw buffer uses dark bg and light text colors.

**Acceptance Scenarios**:

1. **Given** the library, **When** app uses the documented dark preset (e.g. `cui_theme_dark()` or `cui_set_theme(ctx, &cui_theme_dark)`), **Then** widgets render with dark backgrounds and light text.
2. **Given** app has set dark theme, **When** app calls `cui_set_theme(ctx, &light_default)` or NULL to reset, **Then** appearance reverts to light default.

---

### User Story 3 — Draw and layout use theme from context (Priority: P1)

All current usages of `CUI_THEME_*` in `draw_cmd.c` and `layout.c` (and any other internal code) read from the context’s current theme instead of compile-time defines. Default theme is applied when no theme is set.

**Why this priority**: Required for User Story 1 to be correct.

**Independent Test**: Covered by US1 test (draw buffer colors) and existing layout/font tests still pass with default theme.

**Acceptance Scenarios**:

1. **Given** context has theme set, **When** `cui_build_draw_from_tree` runs, **Then** it uses `ctx->theme` (or accessor) for button bg, checkbox bg, input bg, text color, focus ring color/width, corner radius.
2. **Given** context has no theme set (or theme is NULL), **When** draw/layout run, **Then** they use built-in default values (no crash; same as current #define values).

---

### Edge Cases

- NULL theme pointer: `cui_set_theme(ctx, NULL)` should reset to built-in default (or no-op if “current” is already default).
- Partial theme: if struct has optional fields (e.g. 0 = use default), document and handle.
- Theme lifetime: app owns the struct; context stores a copy or pointer. If pointer, app must keep it valid for context lifetime. Copy is simpler and avoids lifetime bugs.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: A `cui_theme` struct MUST exist with fields for: default text color, button bg, checkbox bg, input bg, corner radius, font size, focus ring color, focus ring width (mapping 1:1 from current `theme.h` #defines).
- **FR-002**: `cui_set_theme(ctx, &theme)` MUST set the context’s current theme. When `theme` is NULL, the context MUST use the built-in default theme.
- **FR-003**: When no theme has been set (or after reset), draw and layout MUST behave as today (built-in default theme).
- **FR-004**: `draw_cmd.c` and `layout.c` (and any other consumers) MUST read theme values from context (or a shared theme accessor) instead of `CUI_THEME_*` macros for runtime values. Macros MAY remain as default initializers or be removed in favor of a single default struct.
- **FR-005**: A dark theme preset MUST be provided (function or constant struct) so apps can apply dark mode in one call.
- **FR-006**: At least one test MUST apply a custom theme, build a frame, and assert that draw command colors (e.g. button bg) match the theme.

### Key Entities

- **cui_theme**: Struct holding colors (0xAARRGGBB), corner radius (float), font size (int), focus ring color and width. Owned/copied by context when set.
- **Built-in default theme**: Light theme with current #define values; used when no theme is set or when `cui_set_theme(ctx, NULL)`.
- **Dark preset**: A `cui_theme` value (or getter) with dark backgrounds and light text.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: App can call `cui_set_theme(ctx, &theme)` and the next frame’s draw output uses the theme’s colors and focus ring.
- **SC-002**: App can apply a shipped dark preset and see dark UI.
- **SC-003**: No compile-time `#define` theme values are used at draw/layout time; all values come from context’s current theme (or default).
- **SC-004**: Unit test applies a theme and asserts draw buffer colors change accordingly; existing tests still pass with default theme.
