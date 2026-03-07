# Research: Font Atlas — Text Measurement for Layout

## R1: Measurement API shape

**Decision**: Add `void cui_font_measure(int font_id, int font_size_px, const char *utf8, float *out_width, float *out_height)` (or return via a small struct) in `atlas.h`. Return width as advance (sum of scaled advances) and height as line height (ascent + descent) in logical pixels. NULL/empty string → *out_width = 0, *out_height = line height of font at size.

**Rationale**: Layout needs width and height per node; output parameters keep the API C11 and avoid allocating. Font size in pixels matches `CUI_THEME_DEFAULT_FONT_SIZE` and layout’s logical pixel space.

**Alternatives considered**:
- Return struct by value: fine in C, but out params are consistent with existing codebase style.
- Separate width/height functions: one call is simpler for layout and avoids loading metrics twice.

## R2: Font metrics implementation — stb_truetype

**Decision**: Use stb_truetype (single-header, public domain, C) vendored in-repo to load a default TTF and compute metrics. No rasterization or atlas texture required for this feature; only `stbtt_InitFont`, `stbtt_ScaleForPixelHeight`, `stbtt_GetFontVMetrics`, `stbtt_GetCodepointHMetrics`, and UTF-8 codepoint iteration.

**Rationale**: Zero external dependencies; works with `-std=c11`; widely used; provides advance width and ascent/descent. Measurement = scale × (sum of advances) and scale × (ascent - descent) for height.

**Alternatives considered**:
- FreeType: more features but external dependency and build complexity; overkill for “start” measurement-only.
- Platform-specific APIs (Core Text, DirectWrite): would break Zero-to-Window and cross-platform single build.
- HarfBuzz-only: shaping is optional per constitution; Latin measurement works without it for this scope.

## R3: Default font source

**Decision**: Bundle a single default TTF in the repo (e.g. under `assets/` or `deps/`) or embed a minimal subset, and load it at first use (lazy init) or during context init. Font data can be read from file or linked as array; stb_truetype accepts a buffer. Prefer one file (e.g. Noto Sans Regular or a small permissive-license font) to keep “start” simple.

**Rationale**: No runtime dependency on system fonts; reproducible layout across platforms. If file read is used, document the path; if embedded, document size impact.

**Alternatives considered**:
- System font: platform-specific paths and licensing; harder to keep Zero-to-Window and consistent metrics.
- Generated/metrics-only font: more work; a real TTF is simpler and prepares for future rasterization.

## R4: Layout integration point

**Decision**: In `node_intrinsic_w` / `node_intrinsic_h` (or a shared helper) in `src/layout/layout.c`, for `CUI_NODE_LABEL`: if node has `label_text`, call `cui_font_measure(cui_font_default_id(), CUI_THEME_DEFAULT_FONT_SIZE, label_text, &w, &h)` and use w/h (with a minimum e.g. 1×line_height for empty string). If `label_text` is NULL or empty, use a small minimum (e.g. 0 width or 1px, height = line height). Layout does not hold font state; it calls font layer each measure pass.

**Rationale**: Keeps font ownership in font layer; layout stays stateless and only consumes the measurement API. Theme font size is in theme.h; layout includes it or receives it via a shared header.

**Alternatives considered**:
- Widget sets layout_w/h before layout: possible but duplicates measure at widget vs layout; measuring in layout single place is simpler.
- Cache measured size on node: can be added later to avoid re-measure when text unchanged; out of scope for “start.”

## R5: UTF-8 and Latin-only for “start”

**Decision**: For this feature, measurement iterates UTF-8 codepoints and uses stb_truetype’s codepoint metrics. No HarfBuzz; no complex shaping. Latin (and any glyphs present in the default TTF) get correct advance; substitution/missing glyph can return 0 advance. Document that full complex-script shaping is a follow-up.

**Rationale**: Unblocks layout for the common case; keeps scope and deps minimal; constitution allows HarfBuzz as optional later.
