# Tasks: Review Fixes

**Input**: Design documents from `/specs/012-review-fixes/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/public-api-changes.md

**Tests**: Included — spec FR-009 explicitly requires expanded test coverage.

**Organization**: Tasks grouped by user story. Bug fixes and improvements from deep repo review.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup

**Purpose**: No new project structure needed — this is a fix/improvement batch on an existing codebase.

- [x] T001 Verify clean build baseline: `make clean && make all && make unit-tests && make integration-tests` (all must pass with zero warnings)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Shared data-model changes that multiple user stories depend on.

**CRITICAL**: US1 depends on T002. US4 depends on T003. US9 depends on T002/T004/T005.

- [x] T002 Add `typedef enum` to `enum cui_cmd_type` and add `cui_draw_buf_push_rounded_rect` declaration in `src/core/draw_cmd.h`
- [x] T003 [P] Change `cui_a11y_entry.state` from `const char *state` to `char state[32]` in `src/core/a11y.h`
- [x] T004 [P] Add `typedef enum` to `enum cui_node_type` in `src/core/node.h`
- [x] T005 [P] Remove unused `CUI_DRAW_BUF_MAX` macro from `src/core/draw_cmd.h`

**Checkpoint**: Foundational type changes complete — all user stories can now proceed.

---

## Phase 3: User Story 1 — Correct Drawing Primitives (Priority: P1) MVP

**Goal**: `cui_draw_circle` emits a round draw command; `cui_draw_buf_push_rounded_rect` is available.

**Independent Test**: Call `cui_draw_circle()` and verify emitted command is `CUI_CMD_ROUNDED_RECT`, not `CUI_CMD_RECT`.

### Implementation for User Story 1

- [x] T006 [US1] Implement `cui_draw_buf_push_rounded_rect` in `src/core/draw_cmd.c` (returns 0 on success, -1 if buffer full; emits `CUI_CMD_ROUNDED_RECT`)
- [x] T007 [US1] Change `cui_draw_circle` in `src/widget/canvas.c` to call `cui_draw_buf_push_rounded_rect` instead of `cui_draw_buf_push_rect`
- [x] T008 [US1] Handle `CUI_CMD_ROUNDED_RECT` in `replay_canvas_buf` in `src/core/draw_cmd.c` (currently only handles RECT, TEXT, LINE)

**Checkpoint**: `cui_draw_circle` now emits `CUI_CMD_ROUNDED_RECT`. `render.c` already handles rounded rects in scaling (verified). `make all` compiles with zero warnings.

---

## Phase 4: User Story 2 — Safe Allocation Failure Handling (Priority: P1)

**Goal**: Allocation failure in container/widget functions does not corrupt the parent stack.

**Independent Test**: Verify that `push_container` returning NULL does not push NULL onto the parent stack.

### Implementation for User Story 2

- [x] T009 [US2] Guard `push_container` in `src/widget/layout.c`: if `cui_node_alloc` returns NULL, return NULL without calling `cui_ctx_push_parent`
- [x] T010 [US2] Guard `cui_scroll` in `src/widget/scroll.c`: if `cui_node_alloc` returns NULL, return early without pushing parent
- [x] T011 [P] [US2] Guard `cui_canvas` in `src/widget/canvas.c`: if `cui_node_alloc` returns NULL, return early without pushing parent or setting canvas node

**Checkpoint**: Allocation failure in any container/widget function is a safe no-op. Existing tests pass.

---

## Phase 5: User Story 3 — Type-Safe Public API (Priority: P1)

**Goal**: `cui_set_platform` and `cui_set_rdi` accept typed pointers; type mismatches produce compiler warnings.

**Independent Test**: Passing `const cui_platform *` to `cui_set_rdi` produces a compiler diagnostic.

### Implementation for User Story 3

- [x] T012 [US3] Add forward declarations for `cui_platform`, `cui_platform_ctx`, `cui_rdi`, `cui_rdi_context` in `include/clearui.h`
- [x] T013 [US3] Change `cui_set_platform` signature from `const void *, void *` to `const cui_platform *, cui_platform_ctx *` in `include/clearui.h`
- [x] T014 [US3] Change `cui_set_rdi` signature from `const void *, void *` to `const cui_rdi *, cui_rdi_context *` in `include/clearui.h`
- [x] T015 [US3] Update `cui_set_platform` implementation in `src/core/context.c` to match new signature (remove casts)
- [x] T016 [US3] Update `cui_set_rdi` implementation in `src/core/context.c` to match new signature (remove casts)
- [x] T017 [US3] Verify all test files and integration tests compile cleanly with new signatures

**Checkpoint**: `make all && make unit-tests && make integration-tests` passes with zero warnings. Type mismatches at call sites would produce warnings.

---

## Phase 6: User Story 4 — Composable Accessibility State (Priority: P2)

**Goal**: Focused checked checkbox reports `"checked focused"`, not just `"focused"`.

**Independent Test**: Build a tree with a focused checked checkbox; verify `cui_a11y_entry.state` contains both tokens.

### Implementation for User Story 4

- [x] T018 [US4] Rewrite `visit` function in `src/core/a11y.c` to compose state tokens into `e->state` buffer using `snprintf` (append "checked" then "focused" with space separator)

### Test for User Story 4

- [x] T019 [US4] Create `tests/unit/test_a11y.c`: test that a checked+focused checkbox produces state containing both "checked" and "focused"; test that unchecked unfocused produces empty state; test label fallback (aria_label > label_text > button_id)

**Checkpoint**: `test_a11y` passes. A11y state is composable.

---

## Phase 7: User Story 5 — Fix `cui_text_input_opts` Type (Priority: P2)

**Goal**: `cui_text_input_opts.placeholder` is `const char *`, not `int`.

**Independent Test**: Code using `.placeholder = "Enter name"` compiles.

### Implementation for User Story 5

- [x] T020 [US5] Change `cui_text_input_opts.placeholder` from `int` to `const char *` in `include/clearui.h`
- [x] T021 [US5] Update `cui_text_input` in `src/widget/text_input.c` to store `opts->placeholder` in the node (even if display is still stubbed)

**Checkpoint**: `make all` compiles. Existing tests pass (zero-init `{0}` is still valid for `const char *`).

---

## Phase 8: User Story 6 — Hit-Test Text Inputs (Priority: P2)

**Goal**: Clicking a text input focuses it, consistent with buttons and checkboxes.

**Independent Test**: Inject click at text input position; verify focus is set.

### Implementation for User Story 6

- [x] T022 [US6] Add `CUI_NODE_TEXT_INPUT` to the hit-test condition in `hit_test_visit` in `src/core/context.c`

**Checkpoint**: Text inputs respond to `cui_inject_click`. Existing tests pass.

---

## Phase 9: User Story 7 — Arena Alignment Validation (Priority: P2)

**Goal**: `cui_arena_alloc_aligned` returns NULL for invalid alignment (0, non-power-of-2).

**Independent Test**: Call with alignment 0 and 3; verify NULL return.

### Implementation for User Story 7

- [x] T023 [US7] Add power-of-2 validation (`align == 0 || (align & (align - 1)) != 0`) at top of `cui_arena_alloc_aligned` in `src/core/arena.c`; return NULL if invalid

### Test for User Story 7

- [x] T024 [US7] Add alignment validation test cases to `tests/unit/test_arena.c`: assert NULL for align=0, align=3, align=5; assert non-NULL for align=1, align=2, align=64

**Checkpoint**: `test_arena` passes with new validation cases.

---

## Phase 10: User Story 8 — Expanded Test Coverage (Priority: P3)

**Goal**: Cover previously untested modules: frame_alloc, draw_cmd push functions, keyboard focus.

**Independent Test**: Each new test file compiles and passes via `make unit-tests`.

### Tests for User Story 8

- [x] T025 [P] [US8] Create `tests/unit/test_frame_alloc.c`: test init, alloc, reset (reuse), grow-on-demand, free; print "PASS"
- [x] T026 [P] [US8] Create `tests/unit/test_draw_cmd.c`: test `cui_draw_buf_init`, `push_rect`, `push_line`, `push_text`, `push_rounded_rect`, `clear`, `fini`; test buffer-full returns -1; print "PASS"
- [x] T027 [P] [US8] Create `tests/unit/test_focus.c`: test keyboard focus via `cui_inject_key` with `CUI_KEY_TAB`, `CUI_KEY_SHIFT_TAB`, `CUI_KEY_ENTER`/`CUI_KEY_SPACE` (activates focused widget); print "PASS"
- [x] T028 [US8] Add `printf("test_arena: PASS\n")` before return in `tests/unit/test_arena.c`
- [x] T029 [US8] Add `printf("test_vault: PASS\n")` before return in `tests/unit/test_vault.c`

### Build Integration for User Story 8

- [x] T030 [US8] Add `test_a11y`, `test_frame_alloc`, `test_draw_cmd`, `test_focus` targets to `Makefile` and add them to `unit-tests` target; add their binaries to `clean` target

**Checkpoint**: `make unit-tests` runs all 10 unit tests (6 existing + 4 new). All print "PASS".

---

## Phase 11: User Story 9 — Clean Up Dead Code and Inconsistencies (Priority: P3)

**Goal**: Remove dead code, add `libclearui.a` target, verify all `CUI_CMD_*` types have push functions.

**Independent Test**: `make lib` produces `libclearui.a`. All `CUI_CMD_*` types (except NOP and COUNT) have corresponding push functions.

### Implementation for User Story 9

- [x] T031 [US9] Add `cui_draw_buf_push_scissor` declaration in `src/core/draw_cmd.h` and implementation in `src/core/draw_cmd.c`
- [x] T032 [US9] Add `lib` target to `Makefile`: `ar rcs libclearui.a $(OBJS)` (excluding platform stub and RDI stub); add to `.PHONY`; add `libclearui.a` to `clean`
- [x] T033 [US9] Update all `enum cui_cmd_type` and `enum cui_node_type` usages in source files that reference the old non-typedef form (if any `enum cui_cmd_type` / `enum cui_node_type` variables exist, update to use the typedef `cui_cmd_type` / `cui_node_type`)

**Checkpoint**: `make lib` produces `libclearui.a`. `make all && make unit-tests && make integration-tests` clean.

---

## Phase 12: Polish & Cross-Cutting Concerns

**Purpose**: Final validation across all changes.

- [x] T034 Run `make clean && make all` — verify zero warnings with `-Wall -Wextra -Wpedantic`
- [x] T035 Run `make unit-tests` — verify all 10 unit tests pass and print "PASS"
- [x] T036 Run `make integration-tests` — verify all 3 integration tests pass (no regressions)
- [x] T037 Run `make lib` — verify `libclearui.a` is produced
- [x] T038 Verify `compile_commands.json` is up to date (or regenerate if needed)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — baseline verification
- **Phase 2 (Foundational)**: Depends on Phase 1 — type changes block US1, US4, US9
- **Phase 3 (US1)**: Depends on T002 (rounded rect declaration)
- **Phase 4 (US2)**: Independent — can run in parallel with US1
- **Phase 5 (US3)**: Independent — can run in parallel with US1/US2
- **Phase 6 (US4)**: Depends on T003 (a11y_entry.state buffer)
- **Phase 7 (US5)**: Independent — can run in parallel with US1-US4
- **Phase 8 (US6)**: Independent — can run in parallel with US1-US5
- **Phase 9 (US7)**: Independent — can run in parallel with US1-US6
- **Phase 10 (US8)**: Depends on US1 (test_draw_cmd needs push_rounded_rect), US4 (test_a11y), US7 (test_arena alignment cases)
- **Phase 11 (US9)**: Depends on T002/T004/T005 (foundational type changes)
- **Phase 12 (Polish)**: Depends on all phases complete

### User Story Dependencies

- **US1 (P1)**: Depends on T002. No dependency on other stories.
- **US2 (P1)**: No dependencies on other stories.
- **US3 (P1)**: No dependencies on other stories.
- **US4 (P2)**: Depends on T003. No dependency on other stories.
- **US5 (P2)**: No dependencies on other stories.
- **US6 (P2)**: No dependencies on other stories.
- **US7 (P2)**: No dependencies on other stories.
- **US8 (P3)**: Depends on US1 (for push_rounded_rect test), US4 (for a11y test), US7 (for arena alignment test).
- **US9 (P3)**: Depends on T002/T004/T005 (foundational).

### Parallel Opportunities

After Phase 2 (foundational), the following can run in parallel:

```
  US1 (draw circle fix)     ─┐
  US2 (alloc failure guard) ─┤
  US3 (type-safe API)       ─┤── All independent, different files
  US5 (text_input_opts)     ─┤
  US6 (hit-test text input) ─┤
  US7 (arena alignment)     ─┘

  US4 (a11y state)          ── Depends on T003 (Phase 2)
  US9 (cleanup)             ── Depends on T002/T004/T005 (Phase 2)
  US8 (test coverage)       ── Depends on US1, US4, US7
```

---

## Parallel Example: Phase 2 Foundational

```
# These 4 tasks touch different files — run in parallel:
T002: src/core/draw_cmd.h (typedef enum + push_rounded_rect decl)
T003: src/core/a11y.h     (state buffer)
T004: src/core/node.h     (typedef enum)
T005: src/core/draw_cmd.h (remove CUI_DRAW_BUF_MAX) — SAME FILE as T002, must be sequential with T002
```

Adjusted: T002 + T005 sequential (same file), T003 and T004 in parallel with them.

## Parallel Example: P1 User Stories

```
# After Phase 2, launch all P1 stories in parallel:
US1: src/core/draw_cmd.c + src/widget/canvas.c
US2: src/widget/layout.c + src/widget/scroll.c + src/widget/canvas.c — OVERLAP with US1 on canvas.c
US3: include/clearui.h + src/core/context.c
```

Adjusted: US1 and US2 share `canvas.c` — run US1 first (T007 in canvas.c), then US2 (T011 in canvas.c). US3 fully parallel with both.

---

## Implementation Strategy

### MVP First (US1 + US2 + US3 only)

1. Complete Phase 1: Verify baseline
2. Complete Phase 2: Foundational type changes
3. Complete Phase 3 (US1): Draw circle fix
4. Complete Phase 4 (US2): Allocation safety
5. Complete Phase 5 (US3): Type-safe API
6. **STOP and VALIDATE**: `make all && make unit-tests && make integration-tests`

### Incremental Delivery

1. P1 stories (US1-US3) → Core bugs fixed
2. P2 stories (US4-US7) → Accessibility, API design, safety improvements
3. P3 stories (US8-US9) → Test coverage, cleanup, build improvements
4. Polish → Full validation

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- All changes must maintain zero warnings under `-Wall -Wextra -Wpedantic`
- Commit after each completed user story
- `render.c` already handles `CUI_CMD_ROUNDED_RECT` scaling — no changes needed there
