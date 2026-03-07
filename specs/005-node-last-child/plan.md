# Implementation Plan: Add `last_child` to `cui_node` for O(1) Appends

**Branch**: `005-node-last-child` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/005-node-last-child/spec.md`

## Summary

Add a `last_child` pointer to `cui_node` so `cui_node_append_child` can append in O(1) instead of walking the sibling chain. The pointer is maintained on the write path only; all read-side traversals (`first_child`/`next_sibling` loops in layout, a11y, draw, diff, hit-test) remain untouched.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None (standard C library only)
**Storage**: N/A
**Testing**: Custom assert-based unit/integration tests via `make unit-tests && make integration-tests`
**Target Platform**: Linux, macOS, Windows (Tier 1)
**Project Type**: Library
**Performance Goals**: 60fps on integrated GPUs; layout sub-microsecond for <1000 nodes
**Constraints**: Zero external dependencies; strict C11; arena-based allocation (no `free`)
**Scale/Scope**: `cui_node` struct + `cui_node_append_child` function + unit test

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Clarity**: `last_child` is a plain pointer field — no macros, no hidden state. API unchanged externally.
- [x] **Ownership**: No ownership change — `last_child` is arena-allocated like all nodes; arena reset clears it. No `free()` added.
- [x] **Zero-to-Window**: No build/dep changes. Single-command compile unaffected.
- [x] **C99/C11**: Pure C11 pointer field. No extensions.
- [x] **Performance**: Eliminates O(n²) append bottleneck → directly supports 60fps target for large trees.
- [x] **Beautiful & Accessible**: No visual/a11y impact — internal data-structure change only.
- [x] **Tech constraints**: No API surface change. RDI/platform untouched.
- [x] **Layers**: Change is entirely in Core layer (`node.h`/`node.c`). No upward dependencies.

## Project Structure

### Documentation (this feature)

```text
specs/005-node-last-child/
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
├── core/
│   ├── node.h           # Add last_child field to cui_node struct
│   └── node.c           # Update cui_node_append_child to use last_child
tests/
└── unit/
    └── test_layout.c    # Add last_child verification test
```

**Structure Decision**: Single-project C library layout. Changes touch exactly 2 source files + 1 test file in the existing structure.

## Complexity Tracking

No constitution violations — table intentionally empty.
