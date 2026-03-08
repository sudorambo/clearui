# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2026-03-08

**ClearUI 1.0.0 — Official Release.**

All nine milestones (0.2.0–0.10.0) complete. 24 unit tests, 5 integration tests, ASan, UBSan, and LeakSanitizer clean on Linux, macOS, and Windows. SDL3 platform backend and software render driver ship. API frozen — no breaking changes until 2.0.

### Added

- **Stable release**: First stable version of ClearUI. All milestones 0.2.0–0.10.0 complete.

### Changed

- **Version**: `CUI_VERSION_MAJOR.MINOR.PATCH` set to 1.0.0. `cui_version_string()` returns `"1.0.0"`.
- **API frozen**: Public API locked as of 0.10.0; no breaking changes until 2.0.

## [0.10.0] - 2026-03-07

### Added

- **Runtime version string**: `cui_version_string(void)` returns the library version at runtime (e.g. `"0.10.0"`), matching `CUI_VERSION_MAJOR.MINOR.PATCH`. Implemented in `src/core/version.c`; not thread-safe.
- **API reference**: `docs/API.md` lists all public functions and types from `clearui.h`, `clearui_platform.h`, and `clearui_rdi.h`. `make docs` prints the path.
- **Migration guide**: `docs/MIGRATION.md` documents breaking and notable API changes from 0.2.0 through 0.10.0 with upgrade notes (e.g. `cui_scroll` id, `poll_events` second arg, `cui_layout_run` ctx).
- **API freeze**: Public API is frozen as of 0.10.0; no breaking changes until 1.0. Documented in README Contributing section.

### Changed

- **Layout padding documentation**: `cui_layout` comment and README now state that `padding_x` / `padding_y` override `padding` when &gt; 0; otherwise `padding` applies to both axes. Behavior unchanged; layout code already matched this contract.
- **Version**: `CUI_VERSION_MAJOR.MINOR.PATCH` set to 0.10.0.

### Fixed

### Deprecated

### Removed

### Security

## [0.9.0] - 2026-03-07

### Added

- **Error callback**: Optional `cui_config.error_callback` and `error_userdata`. When a limit is exceeded (parent stack, focusable list, a11y list, widget ID truncation), the callback is invoked with an error code (`CUI_ERR_PARENT_STACK`, `CUI_ERR_FOCUSABLE_FULL`, `CUI_ERR_A11Y_FULL`, `CUI_ERR_ID_TRUNCATED`, `CUI_ERR_UNBALANCED`). NULL callback = silent truncation (unchanged behavior).
- **CUI_DEBUG**: Define when building to enable assertions: parent stack overflow and unbalanced push/pop at end of frame. Leave undefined for release.
- **UTF-8 robustness**: Shared decoder in `src/core/utf8.c` / `utf8.h` (`cui_utf8_next`, `cui_utf8_next_len`). Rejects overlong encodings and UTF-16 surrogates; no read past buffer. Used by font atlas and software RDI. Unit test `test_utf8`; fuzz target `make fuzz-utf8`.
- **Fuzz targets**: `make fuzz-vault` and `make fuzz-frame` for vault and frame allocator (standalone harnesses; use Clang + `-fsanitize=fuzzer` for libFuzzer).
- **Stress test**: `make stress` runs 1200 widgets × 10 frames; no crash.
- **Leak check**: `make leak-check` runs unit tests under Valgrind (`--leak-check=full --errors-for-leak-kinds=definite`). `make leak-check-lsan` runs tests with LeakSanitizer when Valgrind fails at startup (e.g. ld.so memcmp redirection). CI job runs leak-check on Linux.

### Changed

- **Arena grow**: `grow()` in `src/core/arena.c` uses malloc + memcpy + free instead of realloc so the old block is always explicitly freed (fixes Valgrind "definitely lost" on some platforms).

### Fixed

- Valgrind leak-check option: use `--errors-for-leak-kinds=definite` (not `definitelylost`) for compatibility with Valgrind 3.25+.

### Deprecated

### Removed

### Security

## [0.8.0] - 2026-03-07

### Added

- **Software RDI**: The built-in `cui_rdi_soft_get()` now rasterizes the draw command buffer to an RGBA framebuffer. Implemented: filled rects, rounded rects, lines (with thickness), and text (via stb_truetype glyph bitmaps from the font atlas). Scissor commands clip subsequent draws; one active scissor per submit.
- **Viewport and present**: `cui_rdi_soft_set_viewport(ctx, width, height)` allocates or resizes the framebuffer; call before submit. `cui_rdi_soft_set_platform(ctx, platform_ctx, platform)` wires the optional `present_software` callback so `present()` can blit the framebuffer to the window.
- **Platform**: Optional `present_software(platform_ctx, rgba, width, height, pitch_bytes)` on `cui_platform` for software RDI blit. SDL3 adapter implements it (renderer + texture, `SDL_UpdateTexture` + `SDL_RenderPresent`); stub leaves it NULL.
- **Font rasterization**: `cui_font_render_glyph` and `cui_font_free_glyph_bitmap` in `src/font/atlas.h` for glyph bitmaps (used by software RDI text).
- **RDI accessor**: `cui_rdi_soft_get_framebuffer(ctx, &rgba, &width, &height)` for tests and inspection.
- **Visual regression**: Unit test `test_rdi_soft` submits a known rect and asserts the framebuffer is non-zero; included in `make unit-tests` (23 tests).
- **Docs**: README **Software RDI** section (viewport, present_software wiring); `deps/README.md` documents default TTF path and software RDI text; `specs/021-render-driver-0-8-0/quickstart.md` and contracts updated.

### Changed

- None (new APIs are additive; existing RDI and platform interfaces extended with optional fields/callbacks).

### Fixed

### Deprecated

### Removed

### Security

## [0.7.0] - 2026-03-07

### Added

- **Platform interface**: `cui_platform` extended with optional `scale_factor_get(ctx)` (Hi-DPI) and `poll_events(platform_ctx, void *cui_ctx)` so adapters can call `cui_inject_mouse_move`, `cui_inject_click`, etc. Cursor shape constants: `CUI_CURSOR_DEFAULT`, `CUI_CURSOR_TEXT`.
- **SDL3 adapter** (optional): build with `WITH_SDL3=1` to compile `cui_platform_sdl3.c` and link the SDL3 platform. `cui_platform_sdl3_get()` returns the adapter; supports window, events (mouse, key, text input, scroll), clipboard, cursor, and `scale_factor_get` via `SDL_GetWindowPixelDensity`. Integration test `test_platform_window` runs a minimal windowed frame when SDL3 is available.
- README: **Bring your own platform** (implementing `cui_platform`) and **SDL3 adapter** (build and test).

### Changed

- **API**: `poll_events(platform_ctx)` → `poll_events(platform_ctx, void *cui_ctx)`. Stub and SDL3 adapters updated; pass ClearUI context for event injection.

### Fixed

- SDL3 adapter: resolved parameter name shadowing in `poll_events` (use `app_ctx` and cast to `cui_ctx *`). Use SDL3 cursor enums `SDL_SYSTEM_CURSOR_TEXT` and `SDL_SYSTEM_CURSOR_DEFAULT` (replacing IBEAM/ARROW). Makefile fallback: when `sdl3-config`/`pkg-config` do not provide link flags, use `-lSDL3` so the SDL3 build links on typical installations.

### Deprecated

### Removed

### Security

## [0.6.0] - 2026-03-07

### Added

- **Runtime theming**: `cui_theme` struct (text_color, button_bg, checkbox_bg, input_bg, corner_radius, font_size, focus_ring_color, focus_ring_width). Set via `cui_set_theme(ctx, &theme)`; pass `NULL` to reset to the built-in default. The library copies the struct; draw and layout use the current theme from context.
- `cui_theme_dark(cui_theme *out)` — fills `*out` with a dark preset (dark backgrounds, light text); call `cui_set_theme(ctx, out)` to apply.
- Unit test `test_theme` — asserts custom theme affects draw command colors and `cui_set_theme(ctx, NULL)` restores default.
- README: new **Theming** section (cui_theme, cui_set_theme, reset, dark preset).

### Changed

- **API**: `cui_layout_run(root, viewport_w, viewport_h)` → `cui_layout_run(ctx, root, viewport_w, viewport_h)`. Layout uses `cui_ctx_theme(ctx)->font_size` when `ctx` is non-NULL; pass `NULL` for headless/tests to use default font size (16).
- Draw path and layout no longer use compile-time `CUI_THEME_*` macros at runtime; all theme reads go through `cui_ctx_theme(ctx)`. `src/core/theme.h` remains only for default initializer and fallback.

### Fixed

### Deprecated

### Removed

### Security

## [0.5.0] - 2026-03-07

### Added

- Unit tests for previously untested public APIs: `test_canvas_draw` (cui_canvas, cui_draw_rect, cui_draw_circle, cui_draw_text and draw buffer contents), `test_label_styled` (cui_label_styled style application), `test_spacer` (cui_spacer layout sizing), `test_wrap` (cui_wrap flow wrapping), `test_stack` (cui_stack z-order overlap), `test_style_stack` (cui_push_style / cui_pop_style nesting and restoration), `test_cui_frame_alloc` (cui_frame_alloc lifetime across frames), `test_scale_buf` (Hi-DPI scale_buf path when scale_factor > 1), `test_edge_cases` (NULL ctx, empty frame). Every public API listed in Milestone 4 is now exercised by at least one test.
- All new unit tests are included in `make unit-tests`, `make asan`, and `make ubsan`.

### Changed

- None (no public API changes).

### Fixed

- Canvas draw commands were not replayed into the main draw buffer when building the draw tree: the condition required `cui_ctx_canvas_node(ctx) == root`, but that is cleared when leaving the canvas block. Replay now runs for any `CUI_NODE_CANVAS` node so canvas content is drawn correctly.

### Deprecated

### Removed

### Security

## [0.4.0] - 2026-03-07

### Added

- `cui_inject_scroll(ctx, dx, dy)` — scroll wheel events applied to the scroll container under the pointer in `cui_end_frame`; offset clamped to `[0, content_height - view_height]`.
- `cui_inject_mouse_move(ctx, x, y)` — set pointer position for hover hit-test and scroll target resolution.
- Hover state: `cui_ctx_hovered_id(ctx)` and `cui_ctx_is_hovered(ctx, id)` so widgets can highlight on hover.
- `cui_scroll(ctx, id, opts)` — scroll now accepts an optional `id` (may be NULL); scroll containers with an id under the pointer receive scroll wheel input.
- `cui_ctx_retained_root(ctx)` — accessor for tests and debugging (retained tree root).
- Unit test `test_scroll` (scroll offset update and clamping) and integration test `test_scroll_region` (multi-frame scroll).
- README: scroll/hover usage and optional `platform->cursor_set` when hovered id changes.

### Changed

- **API**: `cui_scroll(ctx, opts)` → `cui_scroll(ctx, id, opts)`. Pass `NULL` for `id` for scroll regions that do not receive wheel events.

### Fixed

### Deprecated

### Removed

### Security

## [0.3.0] - 2026-03-07

### Added

- `cui_inject_char(ctx, codepoint)` — insert one printable character (ASCII 32–126) into the focused text input at the cursor; processed in `cui_end_frame`.
- `CUI_KEY_BACKSPACE` (0x0104) and `CUI_KEY_DELETE` (0x0105) for text editing; when the focused widget is a text input, remove the character before or at the cursor.
- `cui_text_input` return value: returns **1** when that widget’s buffer was modified in the previous frame (insert or backspace/delete), **0** otherwise.
- Cursor position for text input stored in the retained node; diff and key handling clamp cursor to `[0, strlen(buffer)]` (and to 0 when the buffer is shortened).
- Unit test `test_text_input` (insert, backspace, delete, buffer full) and integration test `test_text_input_edit` (type and changed-return).

### Changed

### Fixed

### Deprecated

### Removed

### Security

## [0.2.0] - 2026-03-07

### Added

- `CHANGELOG.md` in [Keep a Changelog](https://keepachangelog.com/) format.
- Documentation in `include/clearui.h` and README for color format (`0xAARRGGBB`), single-threaded contract, and fixed limits (silent truncation).
- Windows job in CI (MSYS2/MinGW); CI runs on Ubuntu, macOS, and Windows.
- ASan and UBSan Makefile targets (`make asan`, `make ubsan`) and a CI job that runs tests under sanitizers.
- `.clang-format` at repo root; Contributing section documents sanitizers and formatting.

### Changed

### Fixed

### Deprecated

### Removed

### Security
