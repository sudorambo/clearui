# Implementation Plan: Release Hygiene (0.2.0)

**Branch**: `015-release-hygiene-0.2.0` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/015-release-hygiene-0.2.0/spec.md`

## Summary

Milestone 1 (0.2.0) adds release hygiene only: no functional code changes. It introduces CHANGELOG (Keep a Changelog), documents color format (`0xAARRGGBB`), single-threaded contract, and fixed limits (with silent-truncation behavior) in the public header and README; adds Windows (MSVC) to GitHub Actions; adds ASan/UBSan build targets and CI step; and adds `.clang-format` for style enforcement. All work is documentation, CI, and tooling.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None (stdlib only; optional vendored stb_truetype for font)  
**Storage**: N/A (no new persistence; CHANGELOG and docs are versioned in repo)  
**Testing**: Existing `make unit-tests` and `make integration-tests`; add sanitizer targets and Windows CI job  
**Target Platform**: Tier 1 — Windows 10+, macOS 12+, Linux (X11/Wayland)  
**Project Type**: Library (declarative immediate-mode GUI)  
**Performance Goals**: 60fps, GPU-friendly pipeline (unchanged)  
**Constraints**: Zero external deps for default build; C11 only; single-threaded API  
**Scale/Scope**: Single codebase; ~60–80 public API functions; docs and CI only this milestone  

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: No hidden allocations, macro magic, or implicit global state; API readable as pseudocode. (No code change; docs clarify existing API.)
- [x] **Ownership**: Memory model (arena / frame allocator / Vault) respected; no `free()` on UI nodes. (Unchanged.)
- [x] **Zero-to-Window**: Build and first-run remain single-command / under 5 min where applicable. (Unchanged; CI and format are additive.)
- [x] **C99/C11**: No C++, no compiler extensions; optional deps documented. (Unchanged; MSVC CI validates C11.)
- [x] **Performance**: 60fps target, GPU-friendly pipeline, layout cost acceptable. (Unchanged.)
- [x] **Beautiful & Accessible**: Defaults and a11y not degraded. (Unchanged.)
- [x] **Tech constraints**: RDI/platform abstraction and API surface cap (~120 functions) respected. (Unchanged.)
- [x] **Layers**: Four-layer architecture; dependencies flow downward only. (Unchanged.)

## Project Structure

### Documentation (this feature)

```text
specs/015-release-hygiene-0.2.0/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (public API / header contract)
└── tasks.md             # Phase 2 output (/speckit.tasks — not created by plan)
```

### Source Code (repository root)

```text
include/
  clearui.h              # public API; add/confirm version macros; document color, threading, limits
  clearui_platform.h
  clearui_rdi.h

examples/
  demo.c

src/
  core/                  # context, arena, vault, diff, draw commands, a11y (limits defined here)
  layout/
  font/
  widget/
  platform/
  rdi/

tests/
  unit/
  integration/

.github/
  workflows/
    ci.yml               # add Windows (MSVC) job; add ASan/UBSan step
```

**Structure Decision**: Single-project layout. Release hygiene touches `include/clearui.h`, `README.md`, new `CHANGELOG.md`, new `.clang-format`, `.github/workflows/ci.yml`, and Makefile (sanitizer targets). No new source directories.

## Complexity Tracking

No constitution violations. This milestone is documentation and CI only; no new complexity.
