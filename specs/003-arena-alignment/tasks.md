# Tasks: Add Default Alignment to Arena Allocator

**Input**: Design documents from `/specs/003-arena-alignment/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/public-api.md, quickstart.md

**Tests**: R4 requires new alignment-specific tests in `test_arena.c`.

**Organization**: Single correctness fix across 2 source files and 1 test file. Requirements R1–R5 map to a linear implementation flow.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which requirement this task fulfills
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: No project setup needed — existing project, existing files

(No tasks — project is already initialized)

---

## Phase 2: Foundational (Implementation)

**Purpose**: Modify the arena allocator to provide default alignment and refactor `cui_arena_alloc_aligned`

- [x] T001 [R1] Add `CUI_ARENA_DEFAULT_ALIGN` constant defined as `_Alignof(max_align_t)` in `src/core/arena.c`
- [x] T002 [R1] Modify `cui_arena_alloc` in `src/core/arena.c` to pad `a->pos` to `CUI_ARENA_DEFAULT_ALIGN` boundary before each allocation (compute pad, account for pad+size in grow check, bump pos by pad, then return aligned pointer)
- [x] T003 [R3] Refactor `cui_arena_alloc_aligned` in `src/core/arena.c` to compute padding and bump `a->pos` directly instead of calling `cui_arena_alloc` for padding bytes (avoids double-alignment overhead after T002)
- [x] T004 [R2] Add doc comment to `cui_arena_alloc` declaration in `src/core/arena.h` stating the `_Alignof(max_align_t)` alignment guarantee

**Checkpoint**: `cui_arena_alloc` returns aligned memory. `cui_arena_alloc_aligned` works without double-padding. API signature unchanged.

---

## Phase 3: Testing (R4)

**Purpose**: Add alignment verification tests and confirm all existing tests pass

- [x] T005 [R4] Add alignment test section to `tests/unit/test_arena.c`: allocate with sizes 1, 37, and `sizeof(void*)`, assert each returned pointer satisfies `((uintptr_t)ptr % _Alignof(max_align_t)) == 0`; add `#include <stdint.h>` for `uintptr_t`
- [x] T006 [R4] Add sequential-alignment test to `tests/unit/test_arena.c`: perform 3+ allocations of varying sizes (e.g. 1, 7, 33) and assert ALL returned pointers are aligned (verifies alignment holds across multiple bumps)
- [x] T007 [R4] Add `cui_arena_alloc_aligned` test to `tests/unit/test_arena.c`: allocate with custom alignment (e.g. 64 bytes) and assert `((uintptr_t)ptr % 64) == 0`

**Checkpoint**: `test_arena` passes with new alignment assertions.

---

## Phase 4: Verification (R4, R5)

**Purpose**: Full build and test suite validation

- [x] T008 [R4] Run `make clean && make all && make unit-tests && make integration-tests` and confirm zero warnings and all tests PASS
- [x] T009 [R5] Verify memory overhead: compare `a.pos` after 100 allocations of `sizeof(cui_node)` between old and new — confirm overhead is bounded by `100 * (_Alignof(max_align_t) - 1)` bytes (add a quick sanity check to test or verify manually)

**Checkpoint**: All acceptance criteria from spec.md verified.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 2 (Implementation)**: No external dependencies — start immediately
  - T001 → T002 (constant must exist before use)
  - T003 depends on T002 (must understand new `cui_arena_alloc` behavior)
  - T004 is independent of T001–T003 (different file), marked for parallel
- **Phase 3 (Testing)**: Depends on Phase 2 completion (implementation must be in place for tests to pass)
  - T005, T006, T007 all edit the same file (`test_arena.c`) — execute sequentially
- **Phase 4 (Verification)**: Depends on Phase 3 completion

### Parallel Opportunities

- T004 (header doc comment in `arena.h`) can run in parallel with T001–T003 (which modify `arena.c`)
- T005, T006, T007 are sequential (same file) but could be written in a single edit pass

---

## Parallel Example

```bash
# Parallel: arena.c implementation + arena.h doc comment
# Agent A: T001 → T002 → T003 in src/core/arena.c
# Agent B: T004 in src/core/arena.h

# Sequential: all test additions in one file
# T005 → T006 → T007 in tests/unit/test_arena.c (single edit pass recommended)

# Sequential: final verification
# T008: make clean && make all && make unit-tests && make integration-tests
# T009: overhead check
```

---

## Implementation Strategy

### Single-Pass Execution

This is a focused correctness fix — all phases should be completed in a single session:

1. Add constant + modify `cui_arena_alloc` + refactor `cui_arena_alloc_aligned` (T001–T003)
2. Add doc comment to header (T004)
3. Add alignment tests (T005–T007)
4. Full build + test verification (T008–T009)

### Commit Guidance

A single commit covering T001–T007 is appropriate. Suggested message:

```
fix: add default alignment to cui_arena_alloc

Align all arena allocations to _Alignof(max_align_t) to prevent UB on
strict-alignment architectures and match malloc's alignment guarantee.
Refactor cui_arena_alloc_aligned to avoid double-padding overhead.
Add alignment verification tests.
```

---

## Notes

- T001–T003 modify only `src/core/arena.c` — sequential within that file
- T004 modifies only `src/core/arena.h` — can run in parallel with T001–T003
- T005–T007 modify only `tests/unit/test_arena.c` — sequential within that file
- No other source files are changed; existing callers (`node.c`) benefit automatically
- Total: 9 tasks across 4 phases
