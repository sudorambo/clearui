# Tasks: Remove Duplicate CUI_DRAW_BUF_MAX in render.c

**Input**: Design documents from `/specs/004-remove-duplicate-draw-buf-max/`
**Prerequisites**: plan.md, spec.md, research.md, quickstart.md

**Tests**: Not requested. Build verification via `make` is the acceptance criterion.

**Organization**: Single one-line deletion with 3 requirements (R1–R3).

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which requirement this task fulfills
- Include exact file paths in descriptions

---

## Phase 1: Implementation (R1)

**Purpose**: Remove the duplicate `#define`

- [x] T001 [R1] Delete `#define CUI_DRAW_BUF_MAX 1024` (line 10) from `src/core/render.c`

**Checkpoint**: `render.c` uses `CUI_DRAW_BUF_MAX` from `draw_cmd.h` only.

---

## Phase 2: Verification (R2, R3)

**Purpose**: Confirm compilation and test correctness

- [x] T002 [R2] Run `grep -n 'CUI_DRAW_BUF_MAX' src/core/render.c` and confirm no local `#define` remains (usage references are expected)
- [x] T003 [R3] Run `make clean && make all && make unit-tests && make integration-tests` and confirm zero warnings and all tests PASS

**Checkpoint**: All acceptance criteria from spec.md verified.

---

## Dependencies & Execution Order

- **Phase 1**: No dependencies — start immediately
- **Phase 2**: Depends on Phase 1

Total: 3 tasks across 2 phases.

---

## Implementation Strategy

Single-pass execution: delete one line, verify build.
