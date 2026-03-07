# Implementation Plan: Start the Font Atlas — Text Measurement for Layout

**Branch**: `007-font-atlas` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/007-font-atlas/spec.md`

## Summary

Implement a minimal font atlas that provides **text measurement** (width/height for a string at a given font and size) and wire the layout engine to use it for label intrinsic sizing. This removes the single biggest blocker for a usable layout engine (content-aware sizing). Atlas texture and SDF rendering remain a follow-up; this plan focuses on metrics only, using a vendored C solution (e.g. stb_truetype) so the default build stays zero external dependencies.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None for default build; font metrics via vendored single-file C (e.g. stb_truetype)
**Storage**: N/A (font data can be embedded or loaded once at init)
**Testing**: Custom assert-based unit/integration tests; new unit test for font measure and layout label sizing
**Target Platform**: Linux, macOS, Windows (Tier 1)
**Project Type**: Library
**Performance Goals**: 60fps; measure cost acceptable for typical frame (measure once per text node during layout)
**Constraints**: Zero mandatory external deps; C11 only; optional HarfBuzz not used in this feature
**Scale/Scope**: Font layer (atlas.c/h), layout (layout.c), label widget if needed; ~3–5 files touched

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Clarity**: Measurement API is explicit (font id, size, string → width, height). No hidden global font state beyond default font id.
- [x] **Ownership**: Font atlas/measurement can use arena or static init; no `free()` on UI nodes. Atlas font data ownership documented.
- [x] **Zero-to-Window**: Build remains single-command; no new mandatory deps. First run unchanged.
- [x] **C99/C11**: stb_truetype (or chosen solution) is C, compiles with `-std=c11`. No C++ or compiler extensions.
- [x] **Performance**: Measure called once per text node per frame; layout already walks tree — acceptable for &lt;1k nodes.
- [x] **Beautiful & Accessible**: Enables correct layout and sizing; no regression to defaults or a11y.
- [x] **Tech constraints**: New API is 1–2 functions (measure, possibly init); well under cap. RDI unchanged.
- [x] **Layers**: Font layer is below Layout and Widget; layout calls font for measurement. Dependencies flow downward.

## Project Structure

### Documentation (this feature)

```text
specs/007-font-atlas/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
src/
├── font/
│   ├── atlas.c          # Implement measure; load default font (e.g. stb_truetype)
│   └── atlas.h          # Declare cui_font_measure, cui_font_default_id
├── layout/
│   └── layout.c         # Use font measure for CUI_NODE_LABEL intrinsic size
└── widget/
    └── label.c          # Optional: set layout_w/h from measure if needed, or leave to layout

# Vendored (if used)
# e.g. deps/stb_truetype.h or include in repo per stb policy
```

**Structure Decision**: Single-project C library. Font layer in `src/font/`; layout in `src/layout/` calls font for measurement. Optional vendored dep under `deps/` or similar; no new top-level projects.

## Complexity Tracking

No constitution violations — table intentionally empty.
