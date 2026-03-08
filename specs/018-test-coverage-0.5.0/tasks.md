# Tasks: Test Coverage (0.5.0)

**Input**: Design documents from `specs/018-test-coverage-0.5.0/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Spec explicitly requests tests for each area (FR-001–FR-007); this milestone delivers test binaries that exercise canvas, draw, label_styled, spacer, wrap, stack, style stack, frame allocator, scale_buf, and edge cases.

**Organization**: Tasks grouped by user story (US1–US5) so each story can be implemented and validated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1–US5)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `tests/unit/`, `Makefile` at repository root

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Confirm design is loadable and aligned with ROADMAP.

- [x] T001 Verify `specs/018-test-coverage-0.5.0/spec.md` and `plan.md` are present and align with ROADMAP Milestone 4 (Test Coverage 0.5.0).

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: None required; tests use existing context, stub platform/RDI, and internal headers. No new library code.

**Checkpoint**: Proceed to user story phases.

---

## Phase 3: User Story 1 — Canvas and draw commands (Priority: P1) — MVP

**Goal**: Unit test verifies `cui_canvas`, `cui_draw_rect`, `cui_draw_circle`, and `cui_draw_text` produce expected draw buffer contents.

**Independent Test**: Build minimal ctx with stub platform/RDI, run frame with canvas and draw calls, after `cui_end_frame` inspect `cui_ctx_draw_buf(ctx)` and/or `cui_ctx_canvas_buf(ctx)`; assert command types (e.g. `CUI_CMD_RECT`, fill_rect, text) and geometry/color per `specs/018-test-coverage-0.5.0/contracts/test-coverage.md`.

### Implementation for User Story 1

- [x] T002 [US1] Create `tests/unit/test_canvas_draw.c`: minimal ctx (stub platform/RDI), `cui_begin_frame`, open canvas with `cui_canvas`, call `cui_draw_rect`, `cui_draw_circle`, `cui_draw_text` with known args, `cui_end`, `cui_end_frame`; assert draw buffer (via `cui_ctx_draw_buf`/`cui_ctx_canvas_buf` and `src/core/draw_cmd.h` types) contains corresponding commands with expected coordinates and colors. Include `../../include/clearui.h`, `../../src/core/context.h`, `../../src/core/draw_cmd.h` as needed. Print "PASS" and exit 0.

**Checkpoint**: `test_canvas_draw` binary runs and passes (after T011 adds Makefile target).

---

## Phase 4: User Story 2 — Styled label and layout widgets (Priority: P1)

**Goal**: Unit tests verify `cui_label_styled`, `cui_spacer`, `cui_wrap`, and `cui_stack`: style on label node, spacer layout_w/h, wrap flow, stack overlap.

**Independent Test**: Each test builds context, runs frame(s), uses `cui_ctx_declared_root(ctx)` and node layout/style fields to assert expected behavior per contracts.

### Implementation for User Story 2

- [x] T003 [P] [US2] Create `tests/unit/test_label_styled.c`: build ctx with platform/RDI stubs, frame with `cui_label_styled(ctx, text, &style)` (e.g. non-zero text_color), `cui_end_frame`; find label node in declared root, assert `node->text_color` (and optional `text_decoration`) matches style. Print "PASS" and exit 0.
- [x] T004 [P] [US2] Create `tests/unit/test_spacer.c`: build ctx, frame with `cui_spacer(ctx, w, h)` (e.g. 100, 50), `cui_end_frame`; find spacer node in declared root, assert `layout_w`/`layout_h` consistent with w, h (or parent constraints). Print "PASS" and exit 0.
- [x] T005 [P] [US2] Create `tests/unit/test_wrap.c`: build ctx, frame with wrap container and multiple children that would overflow one row; `cui_end_frame`; assert children have positions indicating flow/wrap (e.g. multiple rows). Print "PASS" and exit 0.
- [x] T006 [P] [US2] Create `tests/unit/test_stack.c`: build ctx, frame with stack container and multiple children; `cui_end_frame`; assert stack children overlap (same or contained layout rect). Print "PASS" and exit 0.

**Checkpoint**: All four tests run and pass after T011.

---

## Phase 5: User Story 3 — Style stack and frame allocator (Priority: P2)

**Goal**: Unit tests verify `cui_push_style`/`cui_pop_style` nesting and restoration, and `cui_frame_alloc` lifetime (reset after `cui_begin_frame`).

**Independent Test**: Style test: push A, push B, pop, assert `cui_ctx_current_style(ctx)` matches A. Frame alloc test: allocate in frame N, `cui_begin_frame` for N+1, do not dereference old pointer; run under ASan to confirm no use-after-free.

### Implementation for User Story 3

- [x] T007 [P] [US3] Create `tests/unit/test_style_stack.c`: build ctx, `cui_begin_frame`, push_style A, push_style B, pop_style, assert `cui_ctx_current_style(ctx)` matches A; optional second case: push A, push B, pop, pop, assert restored. Print "PASS" and exit 0.
- [x] T008 [US3] Create `tests/unit/test_cui_frame_alloc.c`: build ctx, frame 1 call `cui_frame_alloc(ctx, size)`, record pointer, use within frame 1 only; `cui_begin_frame` for frame 2, do not dereference old pointer; run second frame and exit. Test must pass under `make asan` (no use-after-free). Print "PASS" and exit 0.

**Checkpoint**: Both tests pass; `test_cui_frame_alloc` included in asan run (T011).

---

## Phase 6: User Story 4 — Hi-DPI scale buffer path (Priority: P2)

**Goal**: Unit test verifies coordinate scaling when `scale_factor > 1` (scale_buf path).

**Independent Test**: Set `cui_config.scale_factor = 2.f`, run frame that produces draw commands with known logical coordinates; after `cui_end_frame` inspect `cui_ctx_scaled_buf(ctx)` and assert coordinates are scaled by 2.

### Implementation for User Story 4

- [x] T009 [US4] Create `tests/unit/test_scale_buf.c`: create ctx with `cui_config.scale_factor = 2.f`, stub platform/RDI; run frame that produces at least one draw with known logical coords (e.g. button or canvas rect at (10, 10)); `cui_end_frame`; assert `cui_ctx_scaled_buf(ctx)` contains commands with scaled coords (e.g. 20, 20). Print "PASS" and exit 0.

**Checkpoint**: `test_scale_buf` passes after T011.

---

## Phase 7: User Story 5 — Edge cases and sanitizers (Priority: P2)

**Goal**: Unit test for NULL ctx, zero-size/empty inputs; ensure all new tests run under ASan/UBSan.

**Independent Test**: Call key public APIs with NULL ctx (expect no crash); zero-size spacer or empty tree (no crash); `make asan` and `make ubsan` run full unit-tests including all new binaries.

### Implementation for User Story 5

- [x] T010 [US5] Create `tests/unit/test_edge_cases.c`: call APIs (e.g. `cui_begin_frame`, `cui_end_frame`, `cui_button`, `cui_label`, `cui_spacer`) with `ctx == NULL` and assert no crash; run empty frame (`cui_begin_frame`, `cui_end_frame` with no widgets) and assert no crash; optional: zero-size spacer or zero capacity. Print "PASS" and exit 0.
- [x] T011 [US5] Add Makefile targets for all 9 new unit tests: `test_canvas_draw`, `test_label_styled`, `test_spacer`, `test_wrap`, `test_stack`, `test_style_stack`, `test_cui_frame_alloc`, `test_scale_buf`, `test_edge_cases` in `Makefile` (pattern: `test_<name>: $(OBJS) tests/unit/test_<name>.c` and link with `$(OBJS) $(LDFLAGS)`). Add all 9 to the `unit-tests` phony target dependency list and to the recipe (run each binary with `&&`). Add all 9 binary names to the `clean` target `rm -f` list. Ensure `make asan` and `make ubsan` run the same unit-tests target so all new tests are included.

**Checkpoint**: `make unit-tests` runs all new tests; `make asan` and `make ubsan` run them with no sanitizer errors.

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Final validation per quickstart.

- [x] T012 Run `make clean && make all && make unit-tests` and optionally `make asan` and `make ubsan`; validate per `specs/018-test-coverage-0.5.0/quickstart.md` that every public API listed in the spec is covered and all tests pass.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1**: No dependencies.
- **Phase 2**: No tasks (foundation is existing code).
- **Phase 3 (US1)**: Depends on Phase 1. Delivers test_canvas_draw.
- **Phase 4 (US2)**: Depends on Phase 1. T003–T006 can run in parallel ([P]).
- **Phase 5 (US3)**: Depends on Phase 1. T007 [P] with T008.
- **Phase 6 (US4)**: Depends on Phase 1.
- **Phase 7 (US5)**: T010 then T011 (T011 wires all 9 tests into Makefile; T010 is one of the 9).
- **Phase 8**: Depends on T011 so all targets exist.

### User Story Dependencies

- **US1 (P1)**: After Phase 1. Independent.
- **US2 (P1)**: After Phase 1. Independent; T003–T006 parallel.
- **US3 (P2)**: After Phase 1. Independent.
- **US4 (P2)**: After Phase 1. Independent.
- **US5 (P2)**: After Phase 1; T011 must run after all other test files exist (T002–T010).

### Parallel Opportunities

- **Phase 4**: T003, T004, T005, T006 (four separate test files) can be implemented in parallel.
- **Phase 5**: T007 and T008 can be done in parallel (different files).
- **Phases 3–6**: T002, T003–T006, T007, T008, T009 can be done in parallel except that T011 needs all test files to exist before adding them to Makefile.

### Implementation Strategy

1. **Phase 1**: T001 (verify spec/plan).
2. **MVP (US1)**: T002 (test_canvas_draw.c). Then T011 to add at least this one to Makefile to run it; or add all 9 in one go after all are written.
3. **Incremental**: T003–T010 (remaining test files). Then T011 (Makefile: all 9 targets + unit-tests + clean). Then T012 (full validate).
4. **Recommended**: Implement T002–T010 (all 9 test files), then T011 (Makefile update for all 9), then T012 (run unit-tests and asan/ubsan).

---

## Notes

- Use existing pattern from `tests/unit/test_focus.c` and `tests/unit/test_scroll.c` (ctx create, stub platform/RDI, begin_frame/end_frame, assert, destroy).
- Tests may include `src/core/context.h`, `src/core/node.h`, `src/core/draw_cmd.h` for internal assertions.
- New test binary `test_cui_frame_alloc` is distinct from existing `test_frame_alloc` (which tests the frame_allocator component in isolation).
- No new public API; no changes to `include/clearui.h` or library source except as needed to support tests (none anticipated).
