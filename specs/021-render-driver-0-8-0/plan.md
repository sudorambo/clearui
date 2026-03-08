# Implementation Plan: Render Driver (0.8.0)

**Branch**: `021-render-driver-0-8-0` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/021-render-driver-0-8-0/spec.md`

## Summary

Implement the software RDI so it rasterizes the draw command buffer (filled rects, rounded rects, lines, text via stb_truetype, scissor clipping) into an RGBA framebuffer and presents it to the platform (blit to surface when available). Add visual regression test infrastructure (render to buffer/PNG, compare). Ship a default TTF or document how to provide one. Technical approach: extend `clearui_rdi_soft.c` with framebuffer allocation, rasterization for each command type, and a present path that uses an optional platform callback for blit; add font doc or bundled TTF; add a test that renders a fixed UI and compares output (checksum or reference PNG).

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: Vendored stb_truetype (existing in font atlas); optional stb_image_write or minimal PNG write for visual regression if not comparing raw buffer checksums. No new mandatory deps.  
**Storage**: N/A (framebuffer and glyph cache in RDI context; no persistent DB).  
**Testing**: Existing Makefile and CI; new unit or integration test that runs software RDI submit with a fixed command buffer, then checks framebuffer contents (checksum or pixel sampling). Optional: render-to-PNG and image diff if infrastructure is lightweight.  
**Target Platform**: Tier 1 (Windows, macOS, Linux); software RDI runs everywhere; present requires platform to provide a blit target (optional callback or extended platform interface).  
**Project Type**: Library (ClearUI); this milestone implements the existing RDI contract in the software driver.  
**Performance Goals**: 60fps target; software rasterization must complete in reasonable time for typical UI (hundreds of draw calls).  
**Constraints**: No C++; RDI init signature may be extended (e.g. optional user_data or dimensions) if needed for present; API surface cap (~120 functions) respected.  
**Scale/Scope**: One software RDI implementation; one default font solution (bundle or doc); one visual regression test or script.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: RDI remains a function-pointer interface; software implementation is a single file; no hidden allocations beyond documented framebuffer/glyph cache.
- [x] **Ownership**: Framebuffer and RDI context owned by soft RDI; allocated in init or first submit, freed in shutdown; no `free()` on UI nodes.
- [x] **Zero-to-Window**: With software RDI + platform adapter, "build, run, see window with UI" remains achievable; default build may still use stub RDI for headless CI unless soft RDI is default.
- [x] **C99/C11**: Implementation in C; stb_truetype is vendored C; no C++.
- [x] **Performance**: Software path targets 60fps for typical UIs; batching and minimal per-draw work; layout unchanged.
- [x] **Beautiful & Accessible**: Rasterized output matches existing draw commands (rects, text); no degradation to theme or a11y (a11y tree unchanged).
- [x] **Tech constraints**: RDI and platform abstraction respected; optional platform extension for software present (e.g. present_software callback) keeps API surface minimal.
- [x] **Layers**: RDI layer only; no layer above RDI; platform may be extended with one optional callback for blit.

## Project Structure

### Documentation (this feature)

```text
specs/021-render-driver-0-8-0/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (software RDI contract)
└── tasks.md             # From /speckit.tasks (not created by plan)
```

### Source Code (repository root)

```text
src/
├── core/                # unchanged
├── layout/
├── widget/
├── font/                # atlas already uses stb_truetype
├── platform/
├── rdi/
│   └── clearui_rdi_soft.c   # extend: framebuffer, rasterize rect/rounded/line/text, scissor, present
└── ...

include/
├── clearui.h
├── clearui_platform.h   # optional: present_software callback for blit
├── clearui_rdi.h       # unchanged or minor extension for soft RDI init
└── ...

deps/
├── stb_truetype.h      # existing
└── (optional) default_font.ttf or README for font path

tests/
├── unit/
│   └── (optional) test_rdi_soft.c  # submit known buffer, checksum framebuffer
└── integration/
    └── (optional) test_visual_regression.c or script (render to PNG, compare)
```

**Structure Decision**: Single project; changes concentrated in `src/rdi/clearui_rdi_soft.c` and optionally `include/clearui_platform.h` for present callback. Font: either add `deps/default_font.ttf` or document in README/deps. Visual regression: test that invokes soft RDI and asserts on buffer checksum or reference image.

## Complexity Tracking

None at this time. If RDI init is extended with a void* user_data or platform callback for present, it will be optional and documented in the contract.
