# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
