# Tasks: Min/Max Constraint Clamping in the Layout Engine

**Input**: Design documents from `/specs/008-min-max-constraint-clamping/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, quickstart.md

## Format: `[ID] [P?] Description`

- **[Rn]**: Requirement from spec (R1–R3)
- Include exact file paths in descriptions

---

## Phase 1: Implementation (R1, R2)

**Purpose**: Add clamp helper and apply it after every layout_w/layout_h assignment so min/max constraints are respected.

- [X] T001 [R1][R2] In `src/layout/layout.c` add a static helper `clamp_node_size(cui_node *n)`: if `n->layout_opts.min_width > 0` set `n->layout_w` to `max(n->layout_w, n->layout_opts.min_width)`; if `n->layout_opts.max_width > 0` set `n->layout_w` to `min(n->layout_w, n->layout_opts.max_width)`; same for `layout_h` with `min_height`/`max_height`. Use 0 as “no constraint” (only clamp when the option is > 0).
- [X] T002 [R1] In `src/layout/layout.c` in the `measure` function, after the switch statement (and in the default branch) when `n->layout_w` or `n->layout_h` has been set, call `clamp_node_size(n)` so every node gets clamped at the end of the measure pass.
- [X] T003 [R1] In `src/layout/layout.c` in `run_layout`, after assigning `n->layout_w` and `n->layout_h` at the start of the function, call `clamp_node_size(n)`; and after each code path that assigns `c->layout_w` or `c->layout_h` for a child `c`, call `clamp_node_size(c)` so children are clamped in the position pass.

---

## Phase 2: Testing (R3)

**Purpose**: Unit test that min/max constraints are applied correctly.

- [X] T004 [R3] In `tests/unit/test_layout.c` add a test section: create a node (e.g. row or column) with `layout_opts.max_width = 100` and an intrinsic or content width larger than 100, run `cui_layout_run`, and assert the node’s final `layout_w == 100`; create another node with `layout_opts.min_height = 40` and intrinsic height less than 40, run layout, and assert final `layout_h == 40`.

---

## Phase 3: Verification

**Purpose**: Full build and test pass.

- [X] T005 [R3] Run `make clean && make all && make unit-tests && make integration-tests` and confirm zero warnings and all tests PASS.

---

## Dependencies & Execution Order

- **Phase 1**: T001 before T002 and T003 (helper must exist before calls). T002 and T003 both touch `layout.c`; do T002 then T003.
- **Phase 2**: Depends on Phase 1 (clamp implemented).
- **Phase 3**: Depends on Phase 1 and 2.

---

## Implementation Strategy

### Execution Order

1. T001 (add `clamp_node_size`)
2. T002 (call in measure)
3. T003 (call in run_layout)
4. T004 (unit test)
5. T005 (full verify)

### Acceptance Criteria (from spec.md)

- Node with `layout_opts.max_width = 100` and computed width 150 has final `layout_w == 100`.
- Node with `layout_opts.min_height = 40` and computed height 20 has final `layout_h == 40`.
- When min/max is 0, no clamping for that bound.
- All tests pass with zero warnings.
