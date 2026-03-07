# Tasks: Scroll & Mouse (0.4.0)

**Input**: Design documents from `specs/017-scroll-mouse-0.4.0/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Requested in spec (FR-006, FR-007, User Story 4); unit and integration test tasks included.

**Organization**: Tasks grouped by user story (US1 = scroll wheel, US2 = hover, US3 = optional cursor, US4 = tests).

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1, US2, US3, US4)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `include/`, `src/`, `tests/` at repository root

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Verify design is loadable.

- [x] T001 Verify `specs/017-scroll-mouse-0.4.0/spec.md` and `plan.md` are present and align with ROADMAP Milestone 3 (scroll & mouse).

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Context fields for mouse/scroll/hover; scroll with id; helpers for scroll target and content height.

**Checkpoint**: Context has mouse_x, mouse_y, hovered_id, pending_scroll_dx/dy; scroll nodes can have id (button_id); hover hit-test and scroll-under-point helpers exist.

- [x] T002 Add `mouse_x`, `mouse_y` (int), `hovered_id` (char array of size `CUI_LAST_CLICKED_ID_MAX`), `pending_scroll_dx`, `pending_scroll_dy` (int) to `struct cui_ctx` in `src/core/context.c`. Initialize hovered_id to empty and clear or leave it each frame per data-model.md; clear pending_scroll_* when consumed in end_frame.
- [x] T003 [P] Extend `cui_scroll` in `src/widget/scroll.c` to accept an optional id: change signature to `void cui_scroll(cui_ctx *ctx, const char *id, const cui_scroll_opts *opts)` (id may be NULL for no scroll target). When id is non-NULL, set the scroll node’s `button_id` to id so it can be looked up for scroll application. Update declaration in `include/clearui.h` and all call sites (e.g. `examples/demo.c`, tests).
- [x] T004 Add a static hover hit-test in `src/core/context.c`: walk the declared tree with `(ctx->mouse_x, ctx->mouse_y)` and set `ctx->hovered_id` to the id of the topmost interactive node (CUI_NODE_BUTTON, CHECKBOX, ICON_BUTTON, TEXT_INPUT) whose layout rect contains the point; depth-first so last match wins. Do not clear hovered_id in begin_frame so app can read it until next hover hit-test.
- [x] T005 Add a static helper in `src/core/context.c` that, given the declared root and (mouse_x, mouse_y), finds the deepest scroll node (CUI_NODE_SCROLL) whose layout rect contains the point and that has a non-NULL `button_id`; return that node or NULL. Use for scroll target in apply_pending_scroll.
- [x] T006 Add a static helper in `src/core/context.c` that, given a scroll node (declared), returns the content height for clamping: sum of children’s `layout_h` plus gap between children (use node’s `layout_opts.gap`, or 0). Return 0 if no children.

---

## Phase 3: User Story 1 — Scroll wheel updates scroll container (Priority: P1) — MVP

**Goal**: App can call `cui_inject_scroll(ctx, dx, dy)`; the scroll container under the pointer receives the delta and offset is clamped.

**Independent Test**: Declare a scroll with id and content taller than viewport; inject mouse move and scroll; assert scroll offset updated and clamped.

### Implementation for User Story 1

- [x] T007 [P] [US1] Declare `void cui_inject_scroll(cui_ctx *ctx, int dx, int dy)` in `include/clearui.h` and document semantics (scroll container under mouse, dy &lt; 0 = scroll down, processed in end_frame) per `specs/017-scroll-mouse-0.4.0/contracts/scroll-hover-api.md`.
- [x] T008 [US1] Implement `cui_inject_scroll` in `src/core/context.c` to set `ctx->pending_scroll_dx = dx` and `ctx->pending_scroll_dy = dy`.
- [x] T009 [US1] In `src/core/context.c` in `cui_end_frame`, after layout and after hover hit-test: if `pending_scroll_dx` or `pending_scroll_dy` is nonzero, use the T005 helper to find the scroll node under (mouse_x, mouse_y). If found and it has `button_id`, use `retained_node_by_id(ctx->retained_root, button_id)` to get the retained scroll node; add `pending_scroll_dy` to its `scroll_offset_y`; compute content height with T006 helper and clamp `scroll_offset_y` to `[0, max(0, content_height - scroll_max_h)]`; then clear `pending_scroll_dx` and `pending_scroll_dy`. If no scroll under mouse or no id, clear pending and no-op.

**Checkpoint**: Scroll container with id under pointer; inject_scroll(0, -40); end_frame; retained scroll node’s scroll_offset_y increased and clamped.

---

## Phase 4: User Story 2 — Mouse hover state tracked and exposed (Priority: P1)

**Goal**: App can call `cui_inject_mouse_move(ctx, x, y)` and query hover (e.g. `cui_ctx_hovered_id`, `cui_ctx_is_hovered`) so widgets can highlight on hover.

**Independent Test**: Declare a button; inject mouse move inside it; after end_frame, assert hovered_id equals button id.

### Implementation for User Story 2

- [x] T010 [P] [US2] Declare `void cui_inject_mouse_move(cui_ctx *ctx, int x, int y)`, `const char *cui_ctx_hovered_id(cui_ctx *ctx)`, and `int cui_ctx_is_hovered(cui_ctx *ctx, const char *id)` in `include/clearui.h`. Declare the two hover query functions in `src/core/context.h` if used by other internal code. Document per contracts.
- [x] T011 [US2] Implement `cui_inject_mouse_move` in `src/core/context.c` to set `ctx->mouse_x = x` and `ctx->mouse_y = y`.
- [x] T012 [US2] In `cui_end_frame` in `src/core/context.c`, after layout and before or after scroll apply, call the hover hit-test (T004) so `ctx->hovered_id` is set for the frame. Ensure hover hit-test runs after layout so layout rects are valid.
- [x] T013 [US2] Implement `cui_ctx_hovered_id` (return `ctx->hovered_id[0] ? ctx->hovered_id : NULL` or the pointer) and `cui_ctx_is_hovered(ctx, id)` (return 1 if id non-NULL and strcmp(hovered_id, id)==0, else 0) in `src/core/context.c`.

**Checkpoint**: inject_mouse_move inside a button; after end_frame, cui_ctx_hovered_id returns that button’s id; cui_ctx_is_hovered(ctx, "btn") returns 1.

---

## Phase 5: User Story 3 — Optional cursor shape (Priority: P2)

**Goal**: Document use of `cui_platform.cursor_set` when hovered_id changes; implementation optional for 0.4.0.

- [x] T014 [P] [US3] Update `README.md` (or public docs) to document that the app may call `platform->cursor_set(platform_ctx, shape)` when `cui_ctx_hovered_id` changes (e.g. pointer over button). No new API in clearui.h required; platform interface already has `cursor_set` in `include/clearui_platform.h`.

**Checkpoint**: Docs describe optional cursor shape usage.

---

## Phase 6: User Story 4 — Tests for scroll and hover (Priority: P2)

**Goal**: Unit test for scroll offset clamping; integration test that scrolls and verifies offset or visible region.

**Independent Test**: Run `make unit-tests` and `make integration-tests`; new tests pass.

### Implementation for User Story 4

- [x] T015 [P] [US4] Create `tests/unit/test_scroll.c`: build minimal ctx, declare one scroll with id and content taller than max_height, inject_mouse_move into scroll rect, inject_scroll, end_frame; assert retained scroll node’s scroll_offset_y is updated and clamped (e.g. not negative, not above content_height - view_height). Print "PASS" and exit 0 per existing unit test style (see `tests/unit/test_focus.c`).
- [ ] T016 [US4] Create `tests/integration/test_scroll_region.c`: run frame loop with a scroll container (with id), inject mouse position inside scroll, inject_scroll several times, end_frame; assert scroll offset (e.g. via state or by reading back through a second frame and checking layout) is in expected range. Print "PASS" and exit 0.
- [x] T017 [US4] Add `test_scroll` and `test_scroll_region` targets to `Makefile` and include them in `unit-tests` and `integration-tests` so `make unit-tests` and `make integration-tests` run the new tests. Update `clean` to remove the new binaries.

**Checkpoint**: `make unit-tests` and `make integration-tests` include and pass the new tests.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Docs and final validation.

- [x] T018 [P] Update `README.md` to mention `cui_inject_scroll`, `cui_inject_mouse_move`, `cui_ctx_hovered_id`, `cui_ctx_is_hovered`, and scroll-with-id usage per `specs/017-scroll-mouse-0.4.0/quickstart.md`.
- [x] T019 Run full `make clean && make all && make unit-tests && make integration-tests` and validate per `specs/017-scroll-mouse-0.4.0/quickstart.md`.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1**: No dependencies.
- **Phase 2**: Depends on Phase 1. T002 (context fields) first; T003 (scroll id) can be [P] with T002; T004–T006 depend on T002 and layout (helpers use ctx and nodes).
- **Phase 3 (US1)**: Depends on Phase 2. T007 declaration [P] with T010; T008–T009 depend on T005, T006, retained_node_by_id.
- **Phase 4 (US2)**: Depends on Phase 2. T010 declaration [P]; T011–T013 depend on T004 (hover hit-test) and T012 (call it in end_frame).
- **Phase 5 (US3)**: Depends on Phase 4 (hover exists). T014 docs only.
- **Phase 6 (US4)**: Depends on Phases 3–4 so scroll and hover work; T015–T017 after T009 and T013.
- **Phase 7**: Depends on Phase 6.

### User Story Dependencies

- **US1 (P1)**: After Phase 2. Delivers: inject_scroll, scroll offset clamped.
- **US2 (P1)**: After Phase 2. Delivers: inject_mouse_move, hover hit-test, hovered_id and query API.
- **US3 (P2)**: After US2. Delivers: docs for optional cursor_set.
- **US4 (P2)**: After US1–US2. Delivers: unit test scroll clamping, integration test scroll region.

### Parallel Opportunities

- **Phase 2**: T002 and T003 can be done in parallel (context vs scroll widget + header).
- **Phase 3–4**: T007 and T010 (header declarations) can be done in parallel.
- **Phase 6**: T015 (unit test) and T016 (integration test) can be written in parallel; T017 (Makefile) after both.
- **Phase 7**: T018 (docs) [P] with T019 (validation).

---

## Implementation Strategy

### MVP First (US1 + US2)

1. Phase 1: T001.
2. Phase 2: T002–T006 (context fields, scroll id, hover hit-test, scroll-under-point, content height).
3. Phase 3: T007–T009 (inject_scroll, apply in end_frame).
4. Phase 4: T010–T013 (inject_mouse_move, hover hit-test in end_frame, hover query API).
5. Validate: Scroll under pointer + inject_scroll; mouse move + hover query.
6. Then add US3 (cursor docs), US4 (tests), Polish.

### Notes

- Scroll application must run **after** layout (so scroll node’s layout rect and children’s layout_h are set) and **after** diff (so retained tree exists). Current end_frame order: process_pending_key, apply_pending_char, **diff**, layout, build_focusable_list, a11y, hit_test_visit (click), build_draw, … So insert hover hit-test and scroll apply **after layout**, and scroll apply needs the **retained** node — so scroll apply must run **after diff** as well. Order is already: diff → layout → … so after layout we have both declared (with layout) and retained. Apply scroll to retained node; next frame diff copies retained scroll_offset_y to declared. Good.
- Content height for clamping uses the **declared** scroll node’s children (after layout); the scroll node we find under mouse is from the declared tree. Retained node is only used to update scroll_offset_y.
