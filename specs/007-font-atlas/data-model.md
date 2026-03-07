# Data Model: Font Atlas (Text Measurement)

## Entity: Font atlas state (internal)

**Location**: `src/font/atlas.c` (file scope or static struct)

- **font_id** (e.g. int): Default font id = 0; returned by `cui_font_default_id()`.
- **font_data** (e.g. stbtt_fontinfo + buffer): Loaded TTF data for the default font; populated at first measure or via explicit init.
- **initialized** (bool/int): Flag so TTF is loaded once; thread-safety out of scope for this feature.

No public struct for “font” yet; the API is function-based (default id + measure).

## Entity: Measurement result (API contract)

**Not a stored entity** — produced by `cui_font_measure`:

- **width** (float): Advance width of the string in logical pixels.
- **height** (float): Line height (ascent + descent) in logical pixels for the given font size.

For NULL or empty string: width = 0 (or minimum); height = line height of font at given size.

## Public API (contract)

| Function | Purpose |
|----------|---------|
| `int cui_font_default_id(void)` | Returns default font id (0). Existing. |
| `void cui_font_measure(int font_id, int font_size_px, const char *utf8, float *out_width, float *out_height)` | Fills out_width and out_height for the given font, size, and UTF-8 string. NULL utf8 → 0 width, theme line height. |

Layout calls `cui_font_measure` with `cui_font_default_id()` and `CUI_THEME_DEFAULT_FONT_SIZE` when computing intrinsic size for `CUI_NODE_LABEL`.

## State transitions

- **First measure or first use**: Load default TTF (from file or embedded buffer); set initialized.
- **Measure**: If not initialized, init then compute; else compute from cached font info. No per-frame state; layout may call every frame.

## Relationships

- **Layout** → **Font**: layout.c calls `cui_font_measure` for label (and optionally other text) nodes. Layout does not store font; font layer owns default font data.
