# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
