# ClearUI Roadmap to 1.0.0

Current version: **0.1.0** (alpha)

This document tracks the work required to reach a stable 1.0.0 release. Items are grouped into milestones roughly ordered by priority. Each milestone can ship as a minor version bump (0.2.0, 0.3.0, ...) on the way to 1.0.

---

## Milestone 1: Release Hygiene (0.2.0)

Foundational items that every release needs. No functional changes.

- [ ] Add `CUI_VERSION_MAJOR/MINOR/PATCH` macros to `clearui.h` (done in 0.1.0 prep)
- [ ] Create `CHANGELOG.md` and adopt [Keep a Changelog](https://keepachangelog.com/) format
- [ ] Document color format (`0xAARRGGBB`) in public header and README
- [ ] Document single-threaded contract in public header and README
- [ ] Document fixed limits and their silent-truncation behavior:
  - `CUI_PARENT_STACK_MAX` (16) — max container nesting depth
  - `CUI_FOCUSABLE_MAX` (64) — max focusable widgets per frame
  - `CUI_A11Y_MAX` (128) — max accessibility entries
  - `CUI_LAST_CLICKED_ID_MAX` (64) — max widget ID length
  - `CUI_FRAME_PRINTF_MAX` (65536) — max formatted string output
- [ ] Add Windows CI (MSVC) to GitHub Actions matrix (README claims MSVC support)
- [ ] Add ASan + UBSan build targets and CI step
- [ ] Add `.clang-format` for consistent style enforcement

---

## Milestone 2: Text Input & Keyboard (0.3.0)

`cui_text_input` is the most visible incomplete feature. Users expect to type into it.

- [ ] Wire keyboard character input through `cui_inject_char` or similar API
- [ ] Implement cursor position tracking in text input
- [ ] Support backspace / delete
- [ ] Make `cui_text_input` return 1 when buffer content changes
- [ ] Add unit tests for text input editing
- [ ] Add integration test: type into field, read back buffer

---

## Milestone 3: Scroll & Mouse (0.4.0)

Scroll containers exist but lack real input. Mouse hover is absent entirely.

- [ ] Add `cui_inject_scroll(ctx, dx, dy)` API for scroll wheel events
- [ ] Wire scroll wheel to `CUI_NODE_SCROLL` offset
- [ ] Clamp scroll offset to content bounds
- [ ] Add mouse hover state tracking (`cui_inject_mouse_move`)
- [ ] Expose hover state to widgets (button highlight on hover)
- [ ] Optional: cursor shape changes via `cui_platform.cursor_set`
- [ ] Add unit tests for scroll offset clamping
- [ ] Add integration test: scroll + verify visible content region

---

## Milestone 4: Test Coverage (0.5.0)

Fill the gaps identified in the readiness audit. Target: every public API function exercised by at least one test.

- [ ] `cui_canvas` + `cui_draw_rect/circle/text` — verify draw buffer contents
- [ ] `cui_label_styled` — verify style application
- [ ] `cui_spacer` — verify layout sizing
- [ ] `cui_wrap` — verify flow wrapping behavior
- [ ] `cui_stack` — verify z-order overlap
- [ ] Hi-DPI `scale_buf` path — verify coordinate scaling at scale_factor > 1
- [ ] `cui_push_style` / `cui_pop_style` — verify nesting and restoration
- [ ] `cui_frame_alloc` — verify lifetime (freed after begin_frame)
- [ ] Edge cases: NULL arguments, zero-size allocations, empty trees
- [ ] Add a Makefile target for ASan/UBSan test runs

---

## Milestone 5: Theming (0.6.0)

`theme.h` is hardcoded `#define`s. A 1.0 library needs runtime theming.

- [ ] Design `cui_theme` struct (colors, radii, font size, focus ring)
- [ ] Add `cui_set_theme(ctx, &theme)` to public API
- [ ] Fall back to built-in defaults when no theme is set
- [ ] Ship a dark theme preset alongside the light default
- [ ] Update draw_cmd.c to read theme from context instead of compile-time defines
- [ ] Add test: apply theme, verify draw command colors change

---

## Milestone 6: Platform Backend (0.7.0)

At least one real platform adapter must ship for the library to be usable.

- [ ] Implement SDL3 platform adapter (`cui_platform_sdl3.c`)
  - Window create/destroy
  - Event polling (mouse, keyboard, scroll, resize)
  - Clipboard get/set
  - Cursor shape
  - Hi-DPI scale factor query
- [ ] Or: implement a native adapter for one OS (X11/Wayland, Win32, Cocoa)
- [ ] Integration test: open a real window, render one frame, close
- [ ] Document how to bring your own platform adapter

---

## Milestone 7: Render Driver (0.8.0)

At least one real RDI backend must ship. The software RDI is currently a no-op.

- [ ] Implement software RDI that rasterizes to an RGBA framebuffer
  - Filled rects, rounded rects, lines
  - Text rendering via stb_truetype glyph bitmaps
  - Scissor clipping
  - Present: blit framebuffer to platform surface
- [ ] Or: implement a GPU RDI (Vulkan, Metal, OpenGL, or WebGPU)
- [ ] Add visual regression test infrastructure (render to PNG, compare)
- [ ] Ship a default TTF font or document how to provide one

---

## Milestone 8: Robustness & Limits (0.9.0)

Harden the library for real-world use.

- [ ] Replace silent truncation with opt-in error callbacks or return codes:
  - Parent stack overflow → return error or assert in debug
  - Focusable/a11y overflow → warn or grow dynamically
  - Widget ID truncation → warn in debug builds
- [ ] Add `CUI_DEBUG` assertions for common misuse (unbalanced push/pop, etc.)
- [ ] Audit `utf8_next` for robustness with malformed input (overlong sequences, surrogates)
- [ ] Fuzz the UTF-8 decoder, vault hash table, and frame allocator
- [ ] Stress test: 1000+ widgets per frame, deep nesting, rapid state churn
- [ ] Memory leak testing (Valgrind / LeakSanitizer CI step)

---

## Milestone 9: API Polish (0.10.0)

Final API review before locking the 1.0 contract.

- [ ] Review every public function signature for consistency and ergonomics
- [ ] Resolve `cui_layout.padding` vs `padding_x`/`padding_y` ambiguity (deprecate or clarify)
- [ ] Consider adding `cui_image` widget (texture + bounds)
- [ ] Consider adding `cui_tooltip` or `cui_popup` container
- [ ] Add `cui_version_string()` runtime function
- [ ] Generate API reference documentation (Doxygen or custom)
- [ ] Write a migration guide for any API changes since 0.1.0
- [ ] Freeze the public API — no breaking changes after this point

---

## 1.0.0 Release Checklist

All of the above milestones complete, plus:

- [ ] All unit and integration tests pass on Linux, macOS, and Windows
- [ ] ASan, UBSan, and Valgrind clean
- [ ] CHANGELOG.md covers every milestone
- [ ] README updated with final API surface
- [ ] At least one real platform backend ships (SDL3 or native)
- [ ] At least one real render driver ships (software framebuffer or GPU)
- [ ] `examples/demo.c` runs with a visible window and renders correctly
- [ ] API reference documentation published
- [ ] Tag `v1.0.0`, create GitHub release

---

## Non-Goals for 1.0

These are explicitly out of scope. They may come in 1.x or 2.0:

- Complex text layout (BiDi, shaping via HarfBuzz)
- Animation / transition system
- Multi-window support
- GPU-accelerated text (SDF atlas)
- Built-in file dialogs or system integration widgets
- Bindings for other languages (Python, Rust, Zig)

---

## How to Contribute

Pick any unchecked item above. Each milestone maps roughly to a feature branch and spec under `specs/`. Before starting, check the existing specs to see if a plan already exists.

```bash
make clean && make all && make unit-tests && make integration-tests
```

All PRs must compile with zero warnings under `-std=c11 -Wall -Wextra -Wpedantic`.
