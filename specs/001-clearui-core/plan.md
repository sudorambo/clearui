# Implementation Plan: ClearUI Core

**Branch**: `001-clearui-core` | **Date**: 2026-03-06 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/001-clearui-core/spec.md`

## Summary

Implement the core of ClearUI: a C99/C11 GUI library with a Declarative Immediate (DI) model—developer writes immediate-mode–style code per frame while the library maintains an internal retained tree for diffing, layout, and accessibility. Primary deliverables: layered architecture (Platform → Core → Layout → Widget), RDI-based rendering, arena + frame allocator + Vault memory model, flexbox-inspired layout, and a minimal public API (~60–80 functions) with zero-to-window in under five minutes. **Zero-to-window (Constitution III)**: MVP delivers a window with a label in Phase 3 (US1); a window with a button is achieved by end of Phase 4 (US2)—together satisfying “single command, window with button in under five minutes.”

## Technical Context

**Language/Version**: C11 (ISO/IEC 9899:2011); compile with `-std=c11 -Wall -Wextra -Wpedantic`.  
**Primary Dependencies**: Optional HarfBuzz for complex-script shaping; default build has zero deps. Platform/Render: one of SDL3 (recommended), GLFW 3.4+, or native (Win32/Cocoa/X11/Wayland). RDI backends: Vulkan 1.1+, Metal, WebGPU, or software fallback.  
**Storage**: N/A (in-memory state only; Vault for persistent UI state, no persistence layer in scope).  
**Testing**: Unit tests (e.g. layout, arena, vault, diff); integration tests for RDI and platform adapters; headless/software RDI for CI.  
**Target Platform**: Tier 1 — Windows 10+, macOS 12+, Linux (X11/Wayland). Tier 2 — iOS 15+, Android 10+, Web (Emscripten).  
**Project Type**: Library (GUI toolkit).  
**Performance Goals**: 60 fps on integrated GPUs; layout pass sub-microsecond for trees &lt;1k nodes; draw command batching via RDI.  
**Constraints**: No C++; no hidden allocations; API surface &lt;~120 functions; single-command build for Hello World.  
**Scale/Scope**: One amalgamated implementation (~25k–40k LOC) or split source; 4 layers; 3 official RDI drivers + 1 soft fallback; 4 platform adapters. GPU drivers (Vulkan, Metal, WebGPU) are out of scope for initial 001-clearui-core; only the software RDI is implemented; GPU drivers are a future phase.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: No hidden allocations, macro magic, or implicit global state; API readable as pseudocode.
- [x] **Ownership**: Memory model (arena / frame allocator / Vault) respected; no `free()` on UI nodes.
- [x] **Zero-to-Window**: Build and first-run remain single-command / under 5 min where applicable.
- [x] **C99/C11**: No C++, no compiler extensions; optional deps (e.g. HarfBuzz) documented.
- [x] **Performance**: 60fps target, GPU-friendly pipeline, layout cost acceptable.
- [x] **Beautiful & Accessible**: Defaults and a11y (keyboard, screen readers, WCAG 2.1 AA) not degraded.
- [x] **Tech constraints**: RDI/platform abstraction and API surface cap (~120 functions) respected.
- [x] **Layers**: Four-layer architecture; dependencies flow downward only.

## Project Structure

### Documentation (this feature)

```text
specs/001-clearui-core/
├── plan.md              # This file
├── research.md          # Phase 0
├── data-model.md        # Phase 1
├── quickstart.md        # Phase 1
├── contracts/           # Phase 1 (RDI, Platform, Public API)
└── tasks.md             # Phase 2 (/speckit.tasks)
```

### Source Code (repository root)

```text
include/
├── clearui.h             # Public API (single header or forward decls)
├── clearui_rdi.h         # RDI struct and function pointer types
└── clearui_platform.h    # Platform adapter interface

src/
├── core/                 # Layer 2: context, node tree, diff, vault, frame alloc, animation
├── layout/               # Layer 3: flexbox engine
├── widget/               # Layer 4: button, label, input, scroll, etc.
├── rdi/                  # RDI implementation + clearui_rdi_soft (headless/CI)
├── platform/             # cui_platform implementation + SDL3 adapter
└── font/                 # SDF atlas, default Noto Sans bundle

rdi/                      # Optional: separate driver repos or subdirs
├── clearui_rdi_vulkan.c
├── clearui_rdi_metal.c
└── clearui_rdi_webgpu.c

tests/
├── unit/                 # Arena, vault, layout, diff
├── integration/          # RDI + platform, Hello World pipeline
└── contract/             # Optional: API contract tests
```

**Structure Decision**: Single-project C library. Primary distribution is two-file drop-in (clearui.h + amalgamated clearui.c); the tree above is the development/reference layout. RDI drivers can live in `src/rdi/` or separate `rdi/` for build isolation. Platform and RDI are behind interfaces so app code does not depend on SDL/Vulkan directly.

## Complexity Tracking

No constitution violations. Complexity Tracking table left empty.
