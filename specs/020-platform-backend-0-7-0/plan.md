# Implementation Plan: Platform Backend (0.7.0)

**Branch**: `020-platform-backend-0-7-0` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/020-platform-backend-0-7-0/spec.md`

## Summary

Ship at least one real platform adapter (SDL3 preferred, or one native OS adapter) so the library can open a window, poll events, and drive ClearUI for real use. Add an integration test that opens a window, renders one frame, and closes. Document how to implement a custom `cui_platform` adapter. Technical approach: implement `cui_platform` in a new file (e.g. `cui_platform_sdl3.c`), keep stub for headless/CI; optionally extend platform interface for scale factor if not already present; add optional integration test (skip when no display); write “bring your own platform” docs.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: SDL3 for the SDL3 adapter (optional dependency; build when SDL3 available) or none for a minimal native adapter. Zero deps for “bring your own” documentation.  
**Storage**: N/A  
**Testing**: Existing Makefile and CI; new integration test (optional run when display available).  
**Target Platform**: Tier 1 (Windows 10+, macOS 12+, Linux X11/Wayland) for the adapter; docs apply to any platform.  
**Project Type**: Library (ClearUI); this milestone adds an optional platform implementation and docs.  
**Performance Goals**: 60 fps target unchanged; event polling and window ops must not block unnecessarily.  
**Constraints**: Single-threaded; app owns event loop; no mandatory CMake/pkg-config for default build (SDL3 can be optional or behind a make flag).  
**Scale/Scope**: One real adapter; one new integration test; one doc section.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: Adapter is a separate file implementing the existing `cui_platform` contract; no hidden state beyond platform context.
- [x] **Ownership**: Platform context is created/destroyed by adapter; ClearUI does not free it; memory model unchanged.
- [x] **Zero-to-Window**: With the new adapter, “download, build, run” still under 5 min when SDL3 (or chosen dep) is available; stub remains for zero-deps headless.
- [x] **C99/C11**: SDL3 adapter uses C and SDL3’s C API; no C++; optional dep documented.
- [x] **Performance**: No change to render path; event polling is platform-normal.
- [x] **Beautiful & Accessible**: No change to theme/a11y; platform delivers input for existing a11y.
- [x] **Tech constraints**: Platform abstraction already in constitution; this implements it; API surface cap unchanged.
- [x] **Layers**: Platform layer only; dependencies flow downward (adapter → platform interface).

## Project Structure

### Documentation (this feature)

```text
specs/020-platform-backend-0-7-0/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (platform adapter contract)
└── tasks.md             # From /speckit.tasks (not created by plan)
```

### Source Code (repository root)

```text
src/
├── core/                # unchanged
├── layout/
├── widget/
├── font/
├── platform/
│   ├── cui_platform_stub.c   # existing; headless/CI
│   └── cui_platform_sdl3.c  # new: SDL3 adapter (or native)
├── rdi/
└── ...

include/
├── clearui.h
├── clearui_platform.h   # existing contract
└── clearui_rdi.h

tests/
├── unit/
└── integration/
    └── test_platform_window.c  # new: open window, one frame, close (optional/skip no-display)
```

**Structure Decision**: Single project; new file under `src/platform/`; new integration test; README or docs section for “bring your own platform”.

## Complexity Tracking

No constitution violations. Optional dependency (SDL3) is documented and build can default to stub when SDL3 not present.
