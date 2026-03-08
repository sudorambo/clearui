# ClearUI Roadmap

**Current version: 1.0.0 (stable)**

ClearUI 1.0.0 has been released. All milestones below are complete. The public API is frozen — no breaking changes until 2.0.

---

## Milestone 1: Release Hygiene (0.2.0) — Done

- [x] Add `CUI_VERSION_MAJOR/MINOR/PATCH` macros to `clearui.h`
- [x] Create `CHANGELOG.md` and adopt [Keep a Changelog](https://keepachangelog.com/) format
- [x] Document color format (`0xAARRGGBB`) in public header and README
- [x] Document single-threaded contract in public header and README
- [x] Document fixed limits and their silent-truncation behavior
- [x] Add Windows CI (MSYS2/MinGW) to GitHub Actions matrix
- [x] Add ASan + UBSan build targets and CI step
- [x] Add `.clang-format` for consistent style enforcement

---

## Milestone 2: Text Input & Keyboard (0.3.0) — Done

- [x] Wire keyboard character input through `cui_inject_char`
- [x] Implement cursor position tracking in text input
- [x] Support backspace / delete
- [x] Make `cui_text_input` return 1 when buffer content changes
- [x] Add unit tests for text input editing
- [x] Add integration test: type into field, read back buffer

---

## Milestone 3: Scroll & Mouse (0.4.0) — Done

- [x] Add `cui_inject_scroll(ctx, dx, dy)` API for scroll wheel events
- [x] Wire scroll wheel to `CUI_NODE_SCROLL` offset
- [x] Clamp scroll offset to content bounds
- [x] Add mouse hover state tracking (`cui_inject_mouse_move`)
- [x] Expose hover state to widgets (button highlight on hover)
- [x] Optional: cursor shape changes via `cui_platform.cursor_set`
- [x] Add unit tests for scroll offset clamping
- [x] Add integration test: scroll + verify visible content region

---

## Milestone 4: Test Coverage (0.5.0) — Done

- [x] `cui_canvas` + `cui_draw_rect/circle/text` — verify draw buffer contents
- [x] `cui_label_styled` — verify style application
- [x] `cui_spacer` — verify layout sizing
- [x] `cui_wrap` — verify flow wrapping behavior
- [x] `cui_stack` — verify z-order overlap
- [x] Hi-DPI `scale_buf` path — verify coordinate scaling at scale_factor > 1
- [x] `cui_push_style` / `cui_pop_style` — verify nesting and restoration
- [x] `cui_frame_alloc` — verify lifetime (freed after begin_frame)
- [x] Edge cases: NULL arguments, zero-size allocations, empty trees
- [x] Add a Makefile target for ASan/UBSan test runs

---

## Milestone 5: Theming (0.6.0) — Done

- [x] Design `cui_theme` struct (colors, radii, font size, focus ring)
- [x] Add `cui_set_theme(ctx, &theme)` to public API
- [x] Fall back to built-in defaults when no theme is set
- [x] Ship a dark theme preset alongside the light default
- [x] Update draw_cmd.c to read theme from context instead of compile-time defines
- [x] Add test: apply theme, verify draw command colors change

---

## Milestone 6: Platform Backend (0.7.0) — Done

- [x] Implement SDL3 platform adapter (`cui_platform_sdl3.c`)
- [x] Integration test: open a real window, render one frame, close
- [x] Document how to bring your own platform adapter

---

## Milestone 7: Render Driver (0.8.0) — Done

- [x] Implement software RDI that rasterizes to an RGBA framebuffer
- [x] Text rendering via stb_truetype glyph bitmaps
- [x] Scissor clipping
- [x] Present: blit framebuffer to platform surface
- [x] Add visual regression test infrastructure
- [x] Ship a default TTF font path and document how to provide one

---

## Milestone 8: Robustness & Limits (0.9.0) — Done

- [x] Replace silent truncation with opt-in error callbacks
- [x] Add `CUI_DEBUG` assertions for common misuse (unbalanced push/pop, etc.)
- [x] Audit and harden UTF-8 decoder (overlong sequences, surrogates)
- [x] Fuzz the UTF-8 decoder, vault hash table, and frame allocator
- [x] Stress test: 1000+ widgets per frame, deep nesting
- [x] Memory leak testing (LeakSanitizer CI step)

---

## Milestone 9: API Polish (0.10.0) — Done

- [x] Review every public function signature for consistency and ergonomics
- [x] Resolve `cui_layout.padding` vs `padding_x`/`padding_y` ambiguity (clarified)
- [x] Add `cui_version_string()` runtime function
- [x] Generate API reference documentation (`docs/API.md`)
- [x] Write a migration guide (`docs/MIGRATION.md`)
- [x] Freeze the public API — no breaking changes after this point

---

## 1.0.0 Release — Done

- [x] All unit and integration tests pass on Linux, macOS, and Windows
- [x] ASan, UBSan, and LeakSanitizer clean
- [x] CHANGELOG.md covers every milestone
- [x] README updated with final API surface
- [x] SDL3 platform backend ships
- [x] Software render driver ships
- [x] `examples/demo.c` runs headlessly and with SDL3
- [x] API reference documentation published
- [x] Tag `v1.0.0`, create GitHub release

---

## Wishlist (post-1.0)

These are not committed — they may come in 1.x or 2.0. Contributions welcome.

- [ ] `cui_image` widget (texture + bounds)
- [ ] `cui_tooltip` or `cui_popup` container
- [ ] Complex text layout (BiDi, shaping via HarfBuzz)
- [ ] Animation / transition system
- [ ] Multi-window support
- [ ] GPU-accelerated text (SDF atlas)
- [ ] GPU render driver (Vulkan, Metal, or WebGPU)
- [ ] Built-in file dialogs or system integration widgets
- [ ] Language bindings (Python, Rust, Zig, Odin)
- [ ] Two-file amalgamated distribution (single header + single .c)

---

## How to Contribute

Pick any wishlist item above, or file an issue with your idea. The API is frozen (additive changes only until 2.0), so new features should extend, not break, the existing surface.

Design documents and feature specs live under `specs/`. The project constitution is at `.specify/memory/constitution.md`.

```bash
make clean && make all && make unit-tests && make integration-tests
```

All PRs must compile with zero warnings under `-std=c11 -Wall -Wextra -Wpedantic`.
