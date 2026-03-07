# Tasks: Text Input & Keyboard (0.3.0)

**Input**: Design documents from `specs/016-text-input-keyboard-0.3.0/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Requested in spec (FR-005, FR-006, User Story 4); unit and integration test tasks included.

**Organization**: Tasks grouped by user story (US1 = type characters, US2 = backspace/delete, US3 = return 1 when changed, US4 = tests).

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1, US2, US3, US4)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `include/`, `src/`, `tests/` at repository root

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: No new repo structure; verify design is loadable.

- [x] T001 Verify `specs/016-text-input-keyboard-0.3.0/spec.md` and `plan.md` are present and align with ROADMAP Milestone 2 (text input & keyboard).

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Cursor storage, context fields, and lookup so US1/US2/US3 can implement insert/delete and return value.

**Checkpoint**: Cursor persists in retained node; context has pending_char and text_input_changed_id; helper to resolve retained node by id exists.

- [x] T002 Add `text_input_cursor` (e.g. `int` or `size_t`) to `struct cui_node` in `src/core/node.h`; document as cursor index in [0, strlen(buffer)] per data-model.md.
- [x] T003 In `src/core/diff.c`: in clone and in `copy_state_retained_to_declared` / copy from declared to retained, copy `text_input_cursor` for `CUI_NODE_TEXT_INPUT` (same pattern as `text_input_buf`/`text_input_cap`).
- [x] T004 Add `pending_char` (unsigned int) and `text_input_changed_id` (char array of size `CUI_LAST_CLICKED_ID_MAX`) to `struct cui_ctx` in `src/core/context.c`. Do not clear `text_input_changed_id` in begin_frame (it is consumed when `cui_text_input` returns 1); in `cui_begin_frame` clear `pending_char` to 0 so each frame has at most one pending character.
- [x] T005 Add a static helper in `src/core/context.c` that walks `ctx->retained_root` to find a node with `button_id` equal to a given id (strcmp); return that node or NULL. Use for resolving the focused text input’s retained node when applying key/char.

---

## Phase 3: User Story 1 — User types characters into a focused text field (Priority: P1) — MVP

**Goal**: User can focus a text input and type printable characters; they are inserted at the cursor and the buffer/display update.

**Independent Test**: Focus the field, call `cui_inject_char` for a sequence of characters; read back the buffer and assert content matches.

### Implementation for User Story 1

- [x] T006 [P] [US1] Declare `void cui_inject_char(cui_ctx *ctx, unsigned int codepoint)` in `include/clearui.h` and document semantics (printable only, focused text input, insert at cursor) per `specs/016-text-input-keyboard-0.3.0/contracts/text-input-api.md`.
- [x] T007 [US1] Implement `cui_inject_char` in `src/core/context.c` to set `ctx->pending_char = codepoint` (no-op if codepoint is not printable, e.g. outside 32–126 for ASCII).
- [x] T008 [US1] In `src/core/context.c` in `cui_end_frame`, after `process_pending_key`: if `ctx->pending_char` is nonzero and the focused widget (by `focusable_ids[focused_index]`) is a text input, use the helper from T005 to get its retained node; insert the character at that node’s `text_input_cursor` into `text_input_buf` (shift tail, advance cursor, NUL-terminate); if buffer full (strlen >= capacity-1) do nothing; set `text_input_changed_id` to that widget id; then clear `pending_char`.
- [x] T009 [US1] In `src/widget/text_input.c` when creating the declared node, set `n->text_input_cursor = 0` so new instances start with cursor at 0; diff will copy cursor from retained when a match exists.

**Checkpoint**: Focusing a text input and calling `cui_inject_char(ctx, 'a')` then `cui_end_frame` results in buffer containing "a".

---

## Phase 4: User Story 2 — User moves cursor and edits with backspace/delete (Priority: P1)

**Goal**: User can use backspace and delete to remove characters; cursor and buffer update correctly.

**Independent Test**: Set buffer to "hello", focus the field, set cursor (e.g. via repeated inject or initial state), inject backspace/delete and assert buffer and cursor.

### Implementation for User Story 2

- [x] T010 [P] [US2] Add `CUI_KEY_BACKSPACE` (0x0104) and `CUI_KEY_DELETE` (0x0105) to `include/clearui.h` per contracts.
- [x] T011 [US2] In `process_pending_key` in `src/core/context.c`: when key is `CUI_KEY_BACKSPACE` or `CUI_KEY_DELETE` and the focused widget is a text input, use the T005 helper to get its retained node; if backspace and cursor > 0, remove character at cursor-1, shift buffer left, decrement cursor; if delete and cursor < strlen(buf), remove character at cursor, shift buffer left; clamp cursor to [0, strlen(buf)]; set `text_input_changed_id` to that widget id. Do not synthesize a click for text input on these keys.

**Checkpoint**: Backspace and delete modify the focused text input’s buffer and cursor; other keys (Tab, Enter, Space) unchanged.

---

## Phase 5: User Story 3 — App knows when the buffer changed (Priority: P2)

**Goal**: `cui_text_input` returns 1 when that widget’s buffer was modified in the previous frame (insert or backspace/delete), 0 otherwise.

**Independent Test**: Call `cui_text_input`, inject a character, `cui_end_frame`, next frame call `cui_text_input` again; return value is 1 for that id.

### Implementation for User Story 3

- [x] T012 [US3] In `src/widget/text_input.c` in `cui_text_input`: before returning, if `ctx->text_input_changed_id[0]` is non-NUL and `strcmp(ctx->text_input_changed_id, id) == 0`, return 1 and clear `ctx->text_input_changed_id` (set to empty string); otherwise return 0. Ensure the widget still registers focus and builds the declared node as today.

**Checkpoint**: After an insert or backspace/delete, the next frame’s `cui_text_input(ctx, id, ...)` for that id returns 1 once, then 0 until the next edit.

---

## Phase 6: User Story 4 — Tests cover text input editing (Priority: P2)

**Goal**: Unit tests for insert, backspace, delete, cursor clamping, buffer full; one integration test that types into a field and reads back the buffer.

**Independent Test**: Run `make unit-tests` and `make integration-tests`; new tests pass.

### Implementation for User Story 4

- [x] T013 [P] [US4] Create `tests/unit/test_text_input.c`: build a minimal ctx, declare one text input, focus it (inject_click or set focus index), call `cui_inject_char` and run end_frame; assert buffer content. Test backspace, delete, cursor clamping after edit, and buffer full (no overflow). Print "PASS" and exit 0 per existing unit test style (see `tests/unit/test_focus.c`).
- [x] T014 [US4] Create `tests/integration/test_text_input_edit.c`: run a short frame loop (begin_frame, declare UI with one text input, end_frame), then inject click to focus, inject chars, end_frame, and assert buffer equals expected string; optionally test backspace/delete. Print "PASS" and exit 0.
- [x] T015 [US4] Add `test_text_input` and `test_text_input_edit` targets to `Makefile` and include them in `unit-tests` and `integration-tests` (or the appropriate phony targets) so `make unit-tests` and `make integration-tests` run the new tests.

**Checkpoint**: `make unit-tests` and `make integration-tests` include and pass the new tests.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Docs and final validation.

- [x] T016 [P] Update `README.md` (or public docs) to mention `cui_inject_char` and `CUI_KEY_BACKSPACE` / `CUI_KEY_DELETE` for text input editing, and that `cui_text_input` returns 1 when content changed.
- [x] T017 Run full `make clean && make all && make unit-tests && make integration-tests` and validate per `specs/016-text-input-keyboard-0.3.0/quickstart.md`.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1**: No dependencies.
- **Phase 2**: Depends on Phase 1. Must complete before any user story (T002–T005).
- **Phase 3 (US1)**: Depends on Phase 2. T006 can be parallel with T010; T007–T009 depend on T005.
- **Phase 4 (US2)**: Depends on Phase 2. T010 can be parallel with T006; T011 depends on T005.
- **Phase 5 (US3)**: Depends on Phase 2 and on T008 (changed_id set on edit). T012 depends on US1/US2 so that edits actually set changed_id.
- **Phase 6 (US4)**: Depends on Phases 3–5 so that insert, backspace, delete, and return value work; T013–T015 can be done after T012.
- **Phase 7**: Depends on Phase 6.

### User Story Dependencies

- **US1 (P1)**: After Phase 2. Delivers: character insertion at cursor.
- **US2 (P1)**: After Phase 2. Delivers: backspace/delete. Can be implemented in parallel with US1 (same context, different keys).
- **US3 (P2)**: After Phase 2 and after edits set `text_input_changed_id` (US1/US2). Delivers: return 1 when changed.
- **US4 (P2)**: After US1–US3. Delivers: unit and integration tests.

### Parallel Opportunities

- **Phase 2**: T002 (node.h) and T004 (context.c) can be done in parallel; T003 and T005 depend on T002 and context layout.
- **Phase 3–4**: T006 (header for inject_char) and T010 (header for key codes) can be done in parallel.
- **Phase 6**: T013 (unit test) and T014 (integration test) can be written in parallel; T015 (Makefile) after both.
- **Phase 7**: T016 (docs) is [P] with T017 (validation).

---

## Parallel Example: User Story 1 + 2 headers

```bash
# Add public API declarations in one edit or two parallel edits:
Task T006: Declare cui_inject_char in include/clearui.h
Task T010: Add CUI_KEY_BACKSPACE and CUI_KEY_DELETE in include/clearui.h
```

---

## Implementation Strategy

### MVP First (User Story 1)

1. Phase 1: T001.
2. Phase 2: T002–T005 (cursor in node, diff, context fields, helper).
3. Phase 3: T006–T009 (cui_inject_char, apply in end_frame, cursor init).
4. **Validate**: Focus text input, inject chars, read buffer.
5. Then add US2 (backspace/delete), US3 (return 1), US4 (tests), Polish.

### Incremental Delivery

1. **MVP**: Phases 1–3 → user can type into focused field.
2. **US2**: Phase 4 → backspace/delete work.
3. **US3**: Phase 5 → return 1 when changed.
4. **US4**: Phase 6 → tests in CI.
5. **Polish**: Phase 7 → docs and full test run.

### Notes

- Insert/backspace/delete must maintain NUL termination and cursor in [0, strlen(buffer)]; clamp after external buffer change is acceptable on next use.
- Buffer full: do not insert when strlen(buffer) >= capacity - 1.
- Non-printable codepoint: cui_inject_char ignores (no-op).
