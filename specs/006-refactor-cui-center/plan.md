# Implementation Plan: Refactor `cui_center` to use `push_container`

**Branch**: `006-refactor-cui-center` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/006-refactor-cui-center/spec.md`

## Summary

Replace the manually duplicated container-setup logic in `cui_center` with a single delegation to the existing `push_container` helper, matching the pattern used by `cui_row`, `cui_column`, `cui_stack`, and `cui_wrap`. Zero behavioral change — pure code deduplication.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None (standard C library only)
**Storage**: N/A
**Testing**: Custom assert-based unit/integration tests via `make unit-tests && make integration-tests`
**Target Platform**: Linux, macOS, Windows (Tier 1)
**Project Type**: Library
**Performance Goals**: No performance impact — same code path, less duplication
**Constraints**: Zero external dependencies; strict C11; public API unchanged
**Scale/Scope**: 1 file, ~15 lines removed, ~2 lines added

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Clarity**: Removes duplication → code is more readable. API unchanged.
- [x] **Ownership**: No ownership/allocation changes — `push_container` uses same arena path.
- [x] **Zero-to-Window**: No build/dep changes.
- [x] **C99/C11**: Pure C11. No extensions.
- [x] **Performance**: Identical codegen — function call instead of inline duplication.
- [x] **Beautiful & Accessible**: No visual/a11y impact.
- [x] **Tech constraints**: No API surface change. RDI/platform untouched.
- [x] **Layers**: Change is entirely in Widget layer (`layout.c`). No layer violations.

## Project Structure

### Documentation (this feature)

```text
specs/006-refactor-cui-center/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0 output
├── quickstart.md        # Phase 1 output
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
src/
└── widget/
    └── layout.c         # Replace cui_center body with push_container call
```

**Structure Decision**: Single file change in existing Widget layer.

## Complexity Tracking

No constitution violations — table intentionally empty.
