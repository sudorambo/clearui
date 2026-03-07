# Feature Spec: Start the Font Atlas — Text Measurement for Layout

**Branch**: `007-font-atlas` | **Date**: 2026-03-07

## Problem

Text measurement is the single biggest blocker for a usable layout engine. Today:

- **Layout** uses hardcoded `LABEL_W 100` and `LABEL_H 20` for every label (`src/layout/layout.c`), so "Hi" and "Hello, World!" get the same box. Columns and rows cannot size correctly to content.
- **Label widget** hardcodes `layout_w = 100`, `layout_h = 20` (`src/widget/label.c`).
- **Font atlas** is a stub: `cui_font_default_id()` returns 0 and there is no measurement API (`src/font/atlas.c`).

Until the layout engine can measure text (width and height for a string at a given font/size), flexible layout and correct wrapping remain impossible.

## Goals

1. **Text measurement API**: The font layer MUST provide a way to measure a UTF-8 string for a given font (id) and font size (logical pixels), returning width and height (e.g. advance width and line height / ascent+descent) so layout can size labels and other text nodes correctly.
2. **Minimal font atlas “start”**: Implement enough of the font atlas to support that measurement — load a default font (e.g. via stb_truetype or similar), expose default font id, and implement measurement. Atlas texture / SDF rasterization and draw-time glyph lookup can follow in a later feature.
3. **Wire layout to measurement**: Layout MUST use measured dimensions for text-carrying nodes (at least `CUI_NODE_LABEL`) when computing intrinsic size, instead of constants `LABEL_W`/`LABEL_H`, so that layout is content-aware.

## Requirements

1. **R1**: Add a text measurement function to the font API (e.g. `cui_font_measure(font_id, font_size_px, utf8_string, out_width, out_height)` or equivalent) that returns width and height in logical pixels for the given string.
2. **R2**: Implement measurement in `src/font/atlas.c` using a real font (e.g. stb_truetype with a bundled or system default TTF). Default font id remains the one returned by `cui_font_default_id()`.
3. **R3**: Layout uses the measurement API for label (and optionally button/text_input) intrinsic size: replace hardcoded `LABEL_W`/`LABEL_H` for `CUI_NODE_LABEL` with measured size from the node’s `label_text` and theme font size; ensure layout still works when `label_text` is NULL or empty (fallback to min size).
4. **R4**: Build remains zero external dependencies by default (stb_truetype vendored or equivalent single-file C solution). Optional HarfBuzz stays optional and out of scope for this “start.”
5. **R5**: All existing tests pass. Add unit test(s) that verify measurement returns sane values for a known string and that layout assigns non-constant sizes to labels when text varies.

## Scope

- **In scope**: Font atlas text measurement API and implementation; layout integration for label (and optionally other text) intrinsic size; unit test for measurement and layout.
- **Out of scope**: Full SDF atlas texture, glyph UVs in draw commands, HarfBuzz, multi-font or font switching API, RDI glyph rendering changes.

## Acceptance Criteria

- `cui_font_measure` (or equivalent) exists and returns width/height for "Hello" and "Hi" such that "Hello" width &gt; "Hi" width.
- Layout assigns different widths to two labels with different text when both use default font/size.
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
- Default build still has zero mandatory external dependencies (beyond C standard library and existing build).
