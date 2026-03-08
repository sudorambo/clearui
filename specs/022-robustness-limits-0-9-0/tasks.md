# Tasks: Robustness & Limits (0.9.0)

**Input**: Design documents from `specs/022-robustness-limits-0-9-0/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1–US5)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Confirm context/config layout and limit constants; no new repo structure.

- [x] T001 Verify `cui_config` and `cui_create` in `include/clearui.h` and `src/core/context.c`; confirm `CUI_PARENT_STACK_MAX`, `CUI_FOCUSABLE_MAX`, `CUI_A11Y_MAX`, `CUI_LAST_CLICKED_ID_MAX` locations and where parent stack push, focusable add, and a11y add occur
- [x] T002 [P] Confirm `utf8_next` (or equivalent) exists in `src/font/atlas.c` and `src/rdi/clearui_rdi_soft.c` and document call sites for shared hardening

---

## Phase 2: Foundational (Error callback and error codes)

**Purpose**: Add optional error callback and error codes so US1 and US2 can invoke it on limit hits. No change to default (silent) behavior until callback is set.

**Checkpoint**: Callback can be set and invoked from one place (e.g. parent stack); release behavior unchanged when callback is NULL.

- [x] T003 Add error callback type and codes: in `include/clearui.h` define `typedef void (*cui_error_fn)(void *userdata, int error_code, const char *limit_name_or_null)` and error code constants (e.g. `CUI_ERR_PARENT_STACK`, `CUI_ERR_FOCUSABLE_FULL`, `CUI_ERR_A11Y_FULL`, `CUI_ERR_ID_TRUNCATED`, `CUI_ERR_UNBALANCED`); document in `specs/022-robustness-limits-0-9-0/contracts/error-callback-0.9.md`
- [x] T004 Extend `cui_config` in `include/clearui.h` with `cui_error_fn error_callback` and `void *error_userdata` (both optional, NULL by default); in `src/core/context.c` store them in `cui_ctx` at create and add an internal helper (e.g. `cui_fire_error(ctx, code, name)`) that calls the callback when non-NULL
- [x] T005 [US1] On parent stack push in `src/core/context.c`: when `parent_top >= CUI_PARENT_STACK_MAX`, call `cui_fire_error(ctx, CUI_ERR_PARENT_STACK, "parent_stack")` (or equivalent) and do not push; keep existing silent behavior when callback is NULL
- [x] T006 [US1] When focusable list is full in `src/core/context.c` (where focusables are registered), call error callback with `CUI_ERR_FOCUSABLE_FULL` and skip adding; when a11y list is full in `src/core/a11y.c`, call error callback with `CUI_ERR_A11Y_FULL` and skip adding (requires context or callback to be available in a11y—e.g. pass ctx to a11y collect or expose a fire-error from context)
- [x] T007 [US1] Where widget IDs are copied into fixed-size buffers (e.g. `CUI_LAST_CLICKED_ID_MAX`), detect truncation and call error callback with `CUI_ERR_ID_TRUNCATED`; document in `include/clearui.h` and contracts

---

## Phase 3: User Story 2 – CUI_DEBUG assertions (P2)

**Goal**: When `CUI_DEBUG` is defined, assert on unbalanced push/pop and parent stack overflow so misuse is caught in development.

**Independent Test**: Build with `-DCUI_DEBUG`, trigger unbalanced pop or excess push; assert fires. Without CUI_DEBUG, no assert.

- [x] T008 [US2] In `src/core/context.c`, wrap parent stack overflow check in `#ifdef CUI_DEBUG` with `assert(parent_top < CUI_PARENT_STACK_MAX)` before push; when over, call error callback if set then assert. Without CUI_DEBUG, only callback (if set) and skip push
- [x] T009 [US2] At end of frame or in `cui_end` (or equivalent), when `CUI_DEBUG` is defined, assert `parent_top == 0` (or document the exact balanced-pop contract and assert there); add assertions for any other common misuse identified in research (e.g. canvas end without begin)
- [x] T010 [P] [US2] Document `CUI_DEBUG` in README or `include/clearui.h`: define for development to enable overflow and balance checks; leave undefined for release

---

## Phase 4: User Story 3 – UTF-8 robustness (P3)

**Goal**: Harden `utf8_next` (or shared helper) so overlong sequences, surrogates, and truncated input do not cause undefined behavior or overread.

**Independent Test**: Unit test with overlong, surrogate, and truncated byte sequences; decoder returns 0 advance or replacement codepoint and does not read past buffer.

- [x] T011 [US3] Add shared UTF-8 helper (e.g. in `src/core/utf8.h` + `utf8.c` or inside `src/font/atlas.c`) that: rejects overlong encodings, rejects or replaces surrogates (U+D800–U+DFFF), and does not read past NUL or past a length bound if a length-bound API is added; return 0 advance on error; document contract in `specs/022-robustness-limits-0-9-0/contracts/utf8-robustness-0.9.md` and in source comments
- [x] T012 [US3] Replace or refactor `utf8_next` in `src/font/atlas.c` to use the hardened helper (or implement the same rules in-place); ensure `cui_font_measure` and any other callers use it
- [x] T013 [US3] Update `src/rdi/clearui_rdi_soft.c` to use the same UTF-8 logic (shared helper or duplicated identical rules) for text drawing; no overread on truncated input
- [x] T014 [P] [US3] Add unit test(s) in `tests/unit/` for UTF-8 decoder: overlong (e.g. C0 80), surrogate range, truncated multi-byte; assert no crash and 0 advance or defined replacement where specified

---

## Phase 5: User Story 4 – Fuzz tests (P4)

**Goal**: Fuzz the UTF-8 decoder (and optionally vault, frame allocator) so regressions are caught.

**Independent Test**: Build fuzz target(s), run with libFuzzer or AFL; no crash on corpus; add to CI or document as optional `make fuzz` job.

- [x] T015 [US4] Add fuzz target for UTF-8 decoder in `tests/fuzz/utf8_fuzz.c` (or equivalent): take bytes from fuzz input, call decoder in a loop until end or error; build with libFuzzer (e.g. `-fsanitize=fuzzer`) or AFL; add Makefile target (e.g. `make fuzz-utf8`) and document in `specs/022-robustness-limits-0-9-0/quickstart.md`
- [x] T016 [P] [US4] Optionally add fuzz targets for vault hash table and frame allocator in `tests/fuzz/`; add to same Makefile and quickstart; run in CI or as separate job if feasible

---

## Phase 6: User Story 5 – Stress tests and memory-leak CI (P5)

**Goal**: Stress test with 1000+ widgets and deep nesting; memory-leak CI with Valgrind or LeakSanitizer.

**Independent Test**: Stress test runs without crash; leak job passes (no leaks in test suite).

- [x] T017 [US5] Add stress test in `tests/stress/` or `tests/unit/`: build a frame with 1000+ widgets (e.g. loop of buttons/labels) and/or deep nesting (e.g. 17 levels); run multiple frames; assert no crash and optionally report timing; add to Makefile
- [x] T018 [US5] Add CI step for memory leaks: e.g. `make leak-check` that runs the test suite under Valgrind (`valgrind --leak-check=full --error-exitcode=1`) or build with LeakSanitizer and run tests; fix any leaks so the job is green; document in `.github/workflows/` or equivalent and in README/Contributing
- [x] T019 [P] [US5] Document stress and leak-check usage in `specs/022-robustness-limits-0-9-0/quickstart.md` and README if appropriate

---

## Phase 7: Polish & Cross-Cutting

**Purpose**: Docs, API summary, and quickstart validation.

- [x] T020 [P] Update `include/clearui.h` fixed-limits comment to mention optional error callback and CUI_DEBUG; ensure all new error codes and callback signature are documented
- [x] T021 [P] Update README with error callback usage, CUI_DEBUG, and (if present) fuzz/stress/leak targets per `specs/022-robustness-limits-0-9-0/quickstart.md`
- [x] T022 Run quickstart steps (set callback, build with CUI_DEBUG, run fuzz/stress/leak if added) and fix any gaps

---

## Phase 8: Follow-up (Leak-check green)

**Purpose**: Ensure `make leak-check` passes so CI job is green.

**Independent Test**: `make leak-check` exits 0; no "definitely lost" or "indirectly lost" from ClearUI code.

- [x] T023 Run `valgrind --leak-check=full ./test_<name> 2>&1` for each unit test to identify which test(s) report leaks; document or fix in the failing test (ensure `cui_destroy`, `cui_vault_destroy`, `cui_frame_allocator_free` on all paths) or in library destroy paths in `src/core/context.c` / `src/core/vault.c` / `src/core/frame_alloc.c`
- [x] T024 Re-run `make leak-check` until it passes; update `specs/022-robustness-limits-0-9-0/follow-up-t016-leak.md` with status

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies.
- **Phase 2 (Foundational)**: Depends on Phase 1. T003–T004 before T005–T007; T005–T007 can be done in order (parent stack, then focusable/a11y, then ID truncation). Blocks US2 (CUI_DEBUG uses same callback and stack).
- **Phase 3 (US2)**: Depends on Phase 2 (context has callback and stack logic). T008–T010.
- **Phase 4 (US3)**: Depends on Phase 1 (utf8 call sites). T011 must be first (shared helper); T012–T013 use it; T014 can run in parallel with T013.
- **Phase 5 (US4)**: Depends on Phase 4 (hardened UTF-8). T015–T016.
- **Phase 6 (US5)**: Largely independent; can start after Phase 1. T017–T019.
- **Phase 7 (Polish)**: After Phases 2–6.
- **Phase 8 (Follow-up)**: After Phase 7. T023–T024 (leak-check green).

### User Story Dependencies

- **US1 (error callback on limits)**: Phase 2 (T003–T007).
- **US2 (CUI_DEBUG)**: Phase 2 done; Phase 3 (T008–T010).
- **US3 (UTF-8)**: Phase 4 (T011–T014).
- **US4 (Fuzz)**: Phase 4 done; Phase 5 (T015–T016).
- **US5 (Stress + leak)**: Phase 6 (T017–T019).

### Parallel Opportunities

- T002 [P] with T001.
- T006 may require passing ctx into a11y or an accessor; T005 and T007 are in context.c and can be sequenced.
- T010 [P], T014 [P], T016 [P], T019 [P], T020 [P], T021 [P] can run in parallel with other tasks in their phase where applicable.

---

## Implementation Strategy

1. **Phase 1**: T001–T002 (verify layout and utf8 sites).
2. **Phase 2**: T003–T004 (callback + helper), then T005 (parent stack), T006 (focusable/a11y), T007 (ID truncation). Resolve a11y callback wiring (ctx in a11y or callback from context).
3. **Phase 3**: T008–T010 (CUI_DEBUG asserts and docs).
4. **Phase 4**: T011 (shared UTF-8 helper), T012–T013 (atlas + rdi), T014 (unit tests).
5. **Phase 5**: T015–T016 (fuzz targets and Makefile).
6. **Phase 6**: T017–T019 (stress test, leak CI, docs).
7. **Phase 7**: T020–T022 (docs and quickstart validation).
8. **Phase 8**: T023–T024 (identify and fix Valgrind leaks so `make leak-check` passes).
