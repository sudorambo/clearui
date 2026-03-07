# Quickstart: Font Atlas Text Measurement

## Font loading

The default font is loaded from `deps/default_font.ttf` (Noto Sans Regular, OFL). The path is resolved at runtime from the current working directory when the first measurement runs. If the file is missing, the atlas falls back to approximate metrics so layout still runs.

## Build & Test

```bash
make clean && make all && make unit-tests && make integration-tests
```

After this feature: default build still has no mandatory external dependencies. New unit test verifies measurement and label sizing.

## What Changed

1. **Font API** (`src/font/atlas.h` / `atlas.c`):
   - `cui_font_measure(font_id, font_size_px, utf8, &width, &height)` returns width and height in logical pixels for a UTF-8 string.
   - Default font is loaded (e.g. via vendored stb_truetype + bundled TTF) so measurement uses real metrics.

2. **Layout** (`src/layout/layout.c`):
   - For `CUI_NODE_LABEL`, intrinsic width/height come from `cui_font_measure` using the node’s `label_text` and theme font size, instead of constants `LABEL_W`/`LABEL_H`.
   - Empty/NULL label uses a minimal height (e.g. line height) and zero or minimal width.

3. **Tests**:
   - Unit test: measure "Hello" and "Hi" and assert "Hello" width > "Hi" width; assert layout assigns different widths to two labels with different text.

## Verification

- `cui_font_measure(cui_font_default_id(), 16, "Hello", &w, &h)` → w > 0, h > 0.
- Two labels "Short" and "Much longer text" in a row/column get different layout widths.
- All existing integration tests (hello, counter, rdi_platform) still pass.
