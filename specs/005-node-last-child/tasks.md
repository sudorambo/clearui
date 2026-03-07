# Tasks: Add `last_child` to `cui_node` for O(1) Appends

**Input**: Design documents from `/specs/005-node-last-child/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, quickstart.md

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Rn]**: Which requirement this task satisfies (R1–R6 from spec.md)
- Include exact file paths in descriptions

---

## Phase 1: Implementation (R1, R2)

**Purpose**: Add the `last_child` field and rewrite `cui_node_append_child` for O(1) appends

- [x] T001 [R1] Add `cui_node *last_child` field to `struct cui_node` between `first_child` and `next_sibling` in `src/core/node.h`
- [x] T002 [R2] Rewrite `cui_node_append_child` in `src/core/node.c` to maintain `last_child` — remove the `while` loop, use `parent->last_child` for O(1) tail append, and update `parent->last_child` on every append

---

## Phase 2: Testing (R6)

**Purpose**: Add unit test verifying `last_child` invariants from data-model.md

- [x] T003 [R6] Add a new test section in `tests/unit/test_layout.c` that appends 3 children (a, b, c) to a parent and asserts: (1) `parent->first_child == a`, (2) `parent->last_child == c`, (3) `a->next_sibling == b`, (4) `b->next_sibling == c`, (5) `c->next_sibling == NULL`, (6) single-child case: `parent->first_child == parent->last_child`

---

## Phase 3: Verification (R3, R4, R5)

**Purpose**: Confirm zero-init, no traversal regressions, and full build/test pass

- [x] T004 [R3] Verify `cui_node_alloc` zero-initializes `last_child` via existing `memset` in `src/core/node.c` — no code change expected, just confirm
- [x] T005 [R4] Verify no read-side traversals reference `last_child` — run `grep -rn 'last_child' src/` and confirm only `node.h` (field) and `node.c` (append logic) appear
- [x] T006 [R5] Run `make clean && make all && make unit-tests && make integration-tests` and confirm zero warnings and all tests PASS

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Implementation)**: No prerequisites — start immediately
  - T001 before T002 (field must exist before append logic uses it)
- **Phase 2 (Testing)**: Depends on Phase 1 completion (T001 + T002)
- **Phase 3 (Verification)**: Depends on Phase 1 + Phase 2

### Parallel Opportunities

- T001 and T003 touch different files (`node.h` vs `test_layout.c`) but T003 needs the `last_child` field from T001, so they are sequential
- T004 and T005 can run in parallel (read-only verification tasks)
- T006 is the final gate — runs after everything else

---

## Implementation Strategy

### Execution Order

1. T001 → T002 (struct field, then append logic)
2. T003 (unit test — should pass immediately after T002)
3. T004 + T005 in parallel (verification — no code changes)
4. T006 (full build + test gate)

### Acceptance Criteria (from spec.md)

- `cui_node_append_child` does not contain a `while` loop
- `parent->last_child` points to the most recently appended child after every append
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings
