# Implementation Plan: Add Default Alignment to Arena Allocator

**Branch**: `003-arena-alignment` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/003-arena-alignment/spec.md`

## Summary

`cui_arena_alloc` returns unaligned memory, risking UB on strict-alignment platforms and performance penalties on x86. The fix adds automatic alignment to `_Alignof(max_align_t)` inside `cui_arena_alloc` without changing its signature. `cui_arena_alloc_aligned` is retained for custom alignment needs and refactored to share alignment logic.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None (zero external dependencies)
**Storage**: N/A
**Testing**: `make unit-tests && make integration-tests` (assert-based C tests)
**Target Platform**: Linux (Ubuntu), macOS (CI matrix); correctness matters on ARM/SPARC
**Project Type**: Library (C GUI library)
**Performance Goals**: Alignment overhead < 15 bytes per allocation (worst case); zero impact on allocation speed
**Constraints**: No ABI break; no public API change; must compile under gcc, clang, MSVC
**Scale/Scope**: 2 source files changed (`arena.c`, `arena.h`), 1 test file updated (`test_arena.c`)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: No API change. Internal alignment logic is straightforward (pad to boundary before bump). No macro sorcery.
- [x] **Ownership**: Arena memory model preserved. Alignment padding is internal; reset/free behavior unchanged. No `free()` on nodes.
- [x] **Zero-to-Window**: No build process change. Same compiler flags, same Makefile.
- [x] **C99/C11**: Uses `_Alignof(max_align_t)` (C11 keyword + `<stddef.h>` type). `<stddef.h>` already included in `arena.h`. No extensions.
- [x] **Performance**: Alignment padding adds at most `_Alignof(max_align_t) - 1` bytes per alloc (typically 7–15 bytes). Arena default is 4 MB; thousands of widgets fit easily. No measurable perf regression.
- [x] **Beautiful & Accessible**: No UI or a11y changes. N/A.
- [x] **Tech constraints**: No API surface change. No RDI/platform impact.
- [x] **Layers**: Change is entirely within Core layer (`src/core/arena.c`). No cross-layer dependency.

**Result**: All gates pass. This is a correctness fix within Core, fully compatible with the constitution.

## Project Structure

### Documentation (this feature)

```text
specs/003-arena-alignment/
├── plan.md              # This file
├── research.md          # Phase 0: alignment strategy research
├── data-model.md        # Phase 1: N/A (no data entities)
├── contracts/
│   └── public-api.md    # Phase 1: arena API contract (unchanged signature)
└── quickstart.md        # Phase 1: verification steps
```

### Source Code (repository root)

```text
src/core/arena.c         # Modified: add alignment to cui_arena_alloc
src/core/arena.h         # Unchanged (API signature preserved)
tests/unit/test_arena.c  # Modified: add alignment verification test
```

## Complexity Tracking

No constitution violations. Table not needed.
