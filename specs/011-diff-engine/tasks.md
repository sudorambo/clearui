# Tasks: Start the Diff Engine

**Input**: Design documents from `/specs/011-diff-engine/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, quickstart.md

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[US1]**: User story 1 — diff engine: structural reconcile, state copy, lifecycle
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: Verify feature branch and baseline build.

- [X] T001 Verify feature branch `011-diff-engine` and that `make clean && make all && make unit-tests && make integration-tests` passes from repo root before making changes.

---

## Phase 2: Foundational (Retained tree teardown)

**Purpose**: Context must free the retained tree on destroy so that when diff allocates retained nodes, there is no leak. Diff module provides the free helper.

- [X] T002 In `src/core/diff.c`: Add a function to free an entire retained subtree (recursive: free children first, then the node; use free() for each node). In `src/core/diff.h`: Declare `void cui_diff_free_retained(cui_node **retained)` (frees `*retained` and sets `*retained = NULL`). In `src/core/context.c`: In `cui_destroy`, before freeing the context, call `cui_diff_free_retained(&ctx->retained_root)` so the retained tree is freed.

---

## Phase 3: User Story 1 — Diff engine (R1, R2, R3)

**Goal**: Implement `cui_diff_run` so that the retained tree mirrors the declared structure, matched nodes reuse retained nodes and have state copied into declared, and orphaned retained nodes are freed.

**Independent Test**: Run the same UI declaration two frames; retained tree after second frame mirrors declared and state (e.g. scroll_offset_y or text input) is preserved. Remove a widget and confirm no leak (e.g. valgrind or unit test that creates/destroys context with changing tree).

- [X] T003 [US1] In `src/core/diff.c`: Add a helper that allocates a new retained node (e.g. `malloc(sizeof(cui_node))`), initializes it from a declared node (copy `type`, `layout_opts`, `label_text`, `button_id`, and other non-state fields from declared; zero or copy state fields as needed). Return the new node; caller will attach it to the retained tree.

- [X] T004 [US1] In `src/core/diff.c`: Implement matching: given one declared node and the list of retained children of the current parent, find the retained node that corresponds to this declared node. Prefer match by `button_id` (if both declared and retained have the same non-empty `button_id`); otherwise match by child index (i-th declared child ↔ i-th retained child). Return the matched retained node or NULL.

- [X] T005 [US1] In `src/core/diff.c`: Add a helper that copies persistent state from a retained node into the corresponding declared node: for `CUI_NODE_SCROLL` copy `scroll_offset_y`; for `CUI_NODE_TEXT_INPUT` copy `text_input_buf` and `text_input_cap` (ensure declared node’s buffer pointer and cap are updated so layout/draw see the retained content); for `CUI_NODE_CHECKBOX` copy `checkbox_checked`. Call this for each matched (declared, retained) pair after reconciliation.

- [X] T006 [US1] In `src/core/diff.c`: Implement `cui_diff_run(cui_node *declared, cui_node **retained)`: (1) If `declared` is NULL, call the free helper on `*retained` and set `*retained = NULL`; return. (2) Otherwise, recursively reconcile: for the root, match or create retained root (reuse existing `*retained` if type matches or create new and free old root); for each child of declared, match (by key or index) or create a new retained child; free any retained child that was not matched. (3) When reusing a retained node, update its fields from declared (type, layout_opts, children structure, etc.) so retained mirrors declared. (4) After building the new retained tree, for each matched pair (declared, retained) call the state-copy helper so declared has persistent state from retained. Ensure layout/draw still run on the declared tree (they already do); no signature change.

---

## Phase 4: Polish & Verification (R4)

**Purpose**: Full build/test pass; optional unit test for diff.

- [X] T007 [P] (Optional) In `tests/unit/test_diff.c` (or existing test file): Add a test that builds a small declared tree (e.g. root + one SCROLL or TEXT_INPUT child), calls `cui_diff_run` twice with the same structure, and asserts the retained tree is non-NULL and has the same structure; optionally set state on the declared node after first run and assert it is preserved after second run (state copy). Wire the test into the Makefile if a new file is created.

- [X] T008 Run `make clean && make all && make unit-tests && make integration-tests` and confirm zero warnings and all tests PASS.

---

## Dependencies & Execution Order

- **Phase 1**: T001 first.
- **Phase 2**: T002 (free helper + context destroy) before Phase 3 so retained tree can be safely torn down when diff starts allocating.
- **Phase 3**: T003 (alloc/init helper) before T006. T004 (matching) before T006. T005 (state copy) before T006. T006 uses T003, T004, T005 and the free helper from T002.
- **Phase 4**: T007 optional; T008 after all implementation tasks.

### Suggested order

1. T001  
2. T002  
3. T003 → T004 → T005  
4. T006  
5. T007 (optional), T008  

---

## Parallel Opportunities

- T003, T004, T005 can be implemented in parallel (different helpers in the same file; implement in sequence to avoid merge conflicts, or do T003 then T004 then T005).
- T007 (optional test) can be written in parallel with T006 if the test is added after T006 is done.

---

## Implementation Strategy

### MVP (User Story 1)

1. T001 (baseline).  
2. T002 (retained free + context destroy).  
3. T003, T004, T005 (helpers).  
4. T006 (cui_diff_run).  
5. T008 (verify).  

### Acceptance (from spec)

- After two frames with the same declaration, retained tree mirrors declared and state is preserved for matched widgets.  
- Removing a widget frees the corresponding retained subtree (no leak).  
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
