# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
