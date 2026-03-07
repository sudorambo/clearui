# Implementation Plan: Min/Max Constraint Clamping in the Layout Engine

**Branch**: `008-min-max-constraint-clamping` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/008-min-max-constraint-clamping/spec.md`

## Summary

Apply the existing `layout_opts.min_width`, `max_width`, `min_height`, `max_height` fields in the layout engine by clamping each node’s `layout_w` and `layout_h` after size computation. Use 0 as “no constraint.” No API or struct changes; implementation-only in `src/layout/layout.c` plus a unit test.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None
**Storage**: N/A
**Testing**: Assert-based unit tests; extend `tests/unit/test_layout.c` for min/max
**Target Platform**: Linux, macOS, Windows (Tier 1)
**Project Type**: Library
**Performance Goals**: 60fps; layout cost unchanged (one clamp per node per dimension)
**Constraints**: No new API; 0 = no constraint
**Scale/Scope**: Single file (layout.c), one helper or inline clamp at each write site

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Clarity**: Clamping is explicit (min/max from layout_opts); no hidden state.
- [x] **Ownership**: No allocation; layout still owns only node fields.
- [x] **Zero-to-Window**: No build/dep changes.
- [x] **C99/C11**: Plain C; no extensions.
- [x] **Performance**: O(1) per node; negligible.
- [x] **Beautiful & Accessible**: Enables constrained layouts; no regression.
- [x] **Tech constraints**: No new functions in public API.
- [x] **Layers**: Change only in Layout layer (`layout.c`).

## Project Structure

### Documentation (this feature)

```text
specs/008-min-max-constraint-clamping/
├── plan.md
├── spec.md
├── research.md
├── data-model.md
├── quickstart.md
└── tasks.md
```

### Source Code (repository root)

```text
src/
└── layout/
    └── layout.c    # Apply clamp after each assignment to layout_w / layout_h

tests/
└── unit/
    └── test_layout.c   # Add test for min/max clamping
```

## Complexity Tracking

No constitution violations.
