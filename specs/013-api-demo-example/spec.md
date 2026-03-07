# Feature Spec: Full API Demo Example

**Branch**: `013-api-demo-example` | **Date**: 2026-03-07

## Problem

Developers and AI agents integrating ClearUI have no single reference that exercises the full public API. The existing examples (hello world, counter) cover ~20% of the surface area. Usage patterns for layout nesting, canvas drawing, accessibility, keyboard navigation, styling, text input, scroll containers, and platform/RDI wiring must be reverse-engineered from tests and headers.

## Solution

Add a comprehensive, commented example file (`examples/demo.c`) that:

1. Wires up platform and RDI (stubs, with comments showing swap points).
2. Demonstrates every widget: `label`, `button`, `checkbox`, `icon_button`, `text_input`, `spacer`.
3. Uses every layout container: `center`, `row`, `column`, `stack`, `wrap`, `scroll`.
4. Shows `cui_state` for persistent state across frames.
5. Shows `cui_frame_printf` and `cui_frame_alloc` for transient per-frame data.
6. Shows `cui_push_style` / `cui_pop_style` for styling.
7. Shows canvas custom drawing (`cui_canvas`, `cui_draw_rect`, `cui_draw_circle`, `cui_draw_text`).
8. Shows accessibility annotations (`cui_aria_label`, `cui_tab_index`).
9. Shows keyboard injection (`cui_inject_key`) for focus navigation.
10. Includes a `make demo` target and a link from the README.

## Requirements

- **R1**: Single file, under 200 lines, compiles and runs with `make demo && ./demo`.
- **R2**: Comments explain *why* each API is used, not just *what* it does.
- **R3**: Runs headlessly (stub platform + software RDI) with deterministic output — no window required.
- **R4**: Prints a summary of what happened (widget states, draw commands issued) so a developer can verify behavior.
- **R5**: Zero warnings under `-std=c11 -Wall -Wextra -Wpedantic`.
- **R6**: No new dependencies.

## Non-goals

- Not a visual showcase (no real window needed).
- Not a tutorial — it's a reference. The quickstart covers tutorial-style learning.
- Not a stress test or benchmark.

## Acceptance Criteria

- [ ] `make demo && ./demo` succeeds with exit code 0.
- [ ] All public API functions from `clearui.h` (except `cui_run` and `cui_dev_overlay`) are called at least once.
- [ ] Comments in the file explain the purpose of each API section.
- [ ] README links to the example.
- [ ] Zero compiler warnings.
