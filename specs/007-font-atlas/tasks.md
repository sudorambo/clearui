# Tasks: Start the Font Atlas — Text Measurement for Layout

**Input**: Design documents from `/specs/007-font-atlas/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, quickstart.md

## Format: `[ID] [P?] [Story?] Description`

- **[Rn]**: Requirement from spec (R1–R5)
- **[US1]**: Single feature deliverable (font measurement + layout integration)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Vendored Deps and Default Font)

**Purpose**: Add stb_truetype and a default TTF so the font layer can load and measure without external build deps.

- [x] T001 Create `deps/` (or equivalent) and add vendored `stb_truetype.h` (single-header C); in `src/font/atlas.c` define `STB_TRUETYPE_IMPLEMENTATION` in one translation unit and `#include` the header so the implementation is compiled once.
- [x] T002 Add a default TTF font file (e.g. Noto Sans Regular or equivalent permissive-license font) under `assets/` or `deps/`; document in `specs/007-font-atlas/quickstart.md` or README how the font is loaded (file path or embedded buffer).

---

## Phase 2: Font Measurement API (R1, R2) [US1]

**Purpose**: Declare and implement `cui_font_measure` and default font loading so layout can query text dimensions.

- [x] T003 [US1] Declare `void cui_font_measure(int font_id, int font_size_px, const char *utf8, float *out_width, float *out_height)` in `src/font/atlas.h`; document NULL utf8 → 0 width, line height for given size.
- [x] T004 [US1] In `src/font/atlas.c` implement default font load (lazy init on first measure): load TTF from path or embedded buffer using stb_truetype; keep `cui_font_default_id()` returning 0.
- [x] T005 [US1] In `src/font/atlas.c` implement `cui_font_measure`: iterate UTF-8 codepoints, use `stbtt_ScaleForPixelHeight`, `stbtt_GetFontVMetrics`, `stbtt_GetCodepointHMetrics`; set `*out_width` to sum of scaled advances, `*out_height` to scaled (ascent - descent); handle NULL or empty string (0 width, height = line height for font_size_px).

---

## Phase 3: Layout Integration (R3) [US1]

**Purpose**: Layout uses measured dimensions for labels instead of hardcoded constants.

- [x] T006 [US1] In `src/layout/layout.c` include `font/atlas.h` and `core/theme.h`; in `node_intrinsic_w` and `node_intrinsic_h` for `CUI_NODE_LABEL` call `cui_font_measure(cui_font_default_id(), CUI_THEME_DEFAULT_FONT_SIZE, n->label_text, &w, &h)` and return measured w/h; for NULL or empty `label_text` use width 0 (or minimal) and height = measured line height.
- [x] T007 [US1] Remove hardcoded `LABEL_W`/`LABEL_H` for the label case in `src/layout/layout.c` (replace with measure-based values from T006); in `src/widget/label.c` stop setting `layout_w = 100` and `layout_h = 20` so layout computes them from measurement (e.g. set to 0 or leave unset so measure pass fills them).

---

## Phase 4: Tests and Verification (R4, R5)

**Purpose**: Unit tests for measurement and content-aware label sizing; full build with zero new external deps.

- [x] T008 [P] [R5] Add `tests/unit/test_font.c`: call `cui_font_measure(cui_font_default_id(), 16, "Hello", &w1, &h1)` and same for `"Hi"`; assert `w1 > w2`, `h1 > 0`, `h2 > 0`; add `test_font` target in `Makefile` that builds and runs it (link `src/font/atlas.c` and any stb implementation).
- [x] T009 [P] [R5] Extend `tests/unit/test_layout.c`: add a test that builds a row (or column) with two labels with different text (e.g. `"Short"` and `"Much longer text"`), runs `cui_layout_run`, and asserts the two label nodes have different `layout_w` (content-aware sizing); ensure `test_layout` in `Makefile` links `src/font/atlas.c` so layout can measure.
- [x] T010 [R4][R5] Run `make clean && make all && make unit-tests && make integration-tests`; confirm zero warnings and all tests PASS; confirm default build has no new mandatory external dependencies (beyond C standard library and vendored stb/font).

---

## Dependencies & Execution Order

- **Phase 1**: No dependencies — do first (T001 then T002).
- **Phase 2**: Depends on Phase 1 (stb + TTF available). T003 → T004 → T005 (declare, then load, then measure).
- **Phase 3**: Depends on Phase 2 (cui_font_measure available). T006 and T007 both touch layout/label; T006 then T007.
- **Phase 4**: Depends on Phase 3. T008 and T009 can be done in parallel (different test files); T010 last.

### Parallel Opportunities

- T008 and T009 are independent test files; can be implemented in parallel after T001–T007.
- T006 and T007 are sequential (layout then label widget).

---

## Implementation Strategy

### Execution Order

1. T001 → T002 (setup)
2. T003 → T004 → T005 (font API)
3. T006 → T007 (layout integration)
4. T008, T009 (tests), then T010 (full verify)

### Acceptance Criteria (from spec.md)

- `cui_font_measure` exists and returns "Hello" width > "Hi" width.
- Layout assigns different widths to two labels with different text.
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
- Default build has zero mandatory external dependencies (vendored stb + bundled TTF only).
