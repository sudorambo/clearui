# Implementation Plan: Start the Diff Engine

**Branch**: `011-diff-engine` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/011-diff-engine/spec.md`

## Summary

Implement the core diff/reconcile logic in `cui_diff_run`: compare the declared tree (frame arena) with the retained tree (persistent storage), produce an updated retained tree that mirrors declared structure, reuse retained nodes when structure matches, copy persistent state (scroll, text input, etc.) so layout/draw see correct state, and free orphaned retained nodes. This is the library's differentiator over pure immediate-mode—explicit ownership of retained nodes (context or dedicated allocator), no hidden magic; matching by key (e.g. button_id) or child index.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None
**Storage**: Declared tree in frame arena (reset each frame). Retained tree in context-owned storage (heap or persistent arena); diff allocates/frees retained nodes.
**Testing**: Existing unit/integration tests; optional unit test for diff (same structure two frames → state preserved)
**Target Platform**: Linux, macOS, Windows (Tier 1)
**Project Type**: Library
**Performance Goals**: 60fps; diff pass O(n) in tree size where feasible; minimal allocs
**Constraints**: C11 only; no new public API; Constitution: app never calls free() on declared nodes (arena); library may free retained nodes when orphaned
**Scale/Scope**: Core diff.c, context.c (retained_root lifecycle); node type and state fields as needed

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: Diff is a single function; matching and state-copy rules documented. No hidden globals.
- [x] **Ownership**: Declared = arena (app never frees). Retained = context-owned; library frees orphaned retained nodes. Explicit.
- [x] **Zero-to-Window**: No change to build or first-run.
- [x] **C99/C11**: No C++; malloc/free for retained nodes or use a dedicated arena.
- [x] **Performance**: Reuse retained nodes when possible; single pass or bounded passes.
- [x] **Beautiful & Accessible**: Enables correct focus/scroll/input state; a11y benefits from stable tree.
- [x] **Tech constraints**: No new public API; diff remains internal entry point.
- [x] **Layers**: Core only (diff, context); layout/draw unchanged in signature.

## Project Structure

### Documentation (this feature)

```text
specs/011-diff-engine/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
src/core/
├── diff.c               # cui_diff_run: reconcile declared vs retained; match, copy state, update retained tree; free orphans
├── diff.h               # Unchanged signature
├── context.c            # retained_root lifecycle (init NULL, pass to diff, free on destroy)
└── node.h               # Optional: key/identity field for matching if not using index-only
```

**Structure Decision**: Single-project C library. Diff lives in Core; context owns retained_root and calls diff each frame. No new layers.

## Complexity Tracking

No constitution violations.
