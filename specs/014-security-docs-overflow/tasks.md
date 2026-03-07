# Tasks: Security and Robustness Hardening (Remaining Recommendations)

**Input**: Design documents from `/specs/014-security-docs-overflow/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: No new test files are required by the spec. Independent test for each story is documentation presence and/or code review; optional unit test for frame_printf length cap (US1) and overflow behavior (US3) may be added.

**Organization**: Tasks are grouped by user story to allow independent implementation and verification.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (US1–US5)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `tests/`, `include/` at repository root (per plan.md)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Confirm feature scope and file list before implementation

- [x] T001 Confirm feature branch 014-security-docs-overflow and file list per plan.md (src/core/context.c, src/core/arena.c, src/core/frame_alloc.c, src/font/atlas.c, include/clearui.h, tests/unit/test_vault.c; vault doc in header or src/core/vault.c)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Single reference task so implementers have the contract text before editing code

**Checkpoint**: Contract doc reviewed — user story implementation can begin

- [x] T002 [P] Review contracts in specs/014-security-docs-overflow/contracts/api-contracts-security.md for implementation reference

---

## Phase 3: User Story 1 – Document frame_printf Format and Allocation Contract (Priority: P1) – MVP

**Goal**: Document that `fmt` must be application-controlled; optionally enforce a maximum formatted length (e.g. 64K) so that unbounded allocation and format-string misuse are avoided.

**Independent Test**: Documentation is present in API comments and/or public docs; if length cap is implemented, a format yielding >64K returns "" and does not allocate unbounded memory.

### Implementation for User Story 1

- [x] T003 [P] [US1] Add API comment to `cui_frame_printf` in include/clearui.h or src/core/context.c stating that `fmt` must be application-controlled (not user input) to avoid format-string risk and unbounded allocation (DoS)
- [x] T004 [US1] (Optional) In src/core/context.c enforce a maximum formatted length (e.g. 65536) in `cui_frame_printf`: if `vsnprintf(NULL, 0, fmt, ap)` returns a value greater than the cap or negative, return "" and do not allocate
- [x] T005 [P] [US1] (Optional) Add unit test in tests/unit/ that when frame_printf length cap is implemented, a format yielding >64K returns "" (skip if T004 not implemented)

**Checkpoint**: User Story 1 is done when documentation exists and, if implemented, the length cap and optional test pass.

---

## Phase 4: User Story 2 – Document UTF-8 String Contract for Font/Measure APIs (Priority: P1)

**Goal**: Document that the UTF-8 parameter to `cui_font_measure` and internal `utf8_next` must be NUL-terminated (valid C string).

**Independent Test**: Documentation states the NUL-termination requirement; code comments at `utf8_next` and `cui_font_measure` reference it.

### Implementation for User Story 2

- [x] T006 [P] [US2] Add comment at `utf8_next` in src/font/atlas.c that `s` must be a valid C string (NUL-terminated) and that truncated buffers cause undefined behavior
- [x] T007 [P] [US2] Add comment to `cui_font_measure` in src/font/atlas.c that `utf8` must be a valid C string (NUL-terminated, valid UTF-8)

**Checkpoint**: User Story 2 is done when both comments are present and review confirms contract clarity.

---

## Phase 5: User Story 3 – Harden Arena and Frame Allocator Growth Against Overflow (Priority: P2)

**Goal**: Before doubling capacity in the grow path, check for integer overflow (e.g. `new_cap <= SIZE_MAX / 2`) and fail allocation instead of performing the multiply.

**Independent Test**: Code review or existing unit tests confirm overflow check; growth fails gracefully when cap would overflow; no regression in normal growth.

### Implementation for User Story 3

- [x] T008 [P] [US3] In src/core/arena.c in the grow path, before `new_cap *= 2`, add check `new_cap <= SIZE_MAX / 2` (include stdint.h if needed); if check fails, do not multiply and do not update capacity so that allocation returns NULL
- [x] T009 [P] [US3] In src/core/frame_alloc.c in the grow path, before `new_cap *= 2`, add check `new_cap <= SIZE_MAX / 2` (include stdint.h if needed); if check fails, do not multiply and return NULL from alloc

**Checkpoint**: User Story 3 is done when both arena and frame allocator have overflow checks and existing tests pass.

---

## Phase 6: User Story 4 – Document Vault Key String Contract (Priority: P2)

**Goal**: Document that the `key` parameter to `cui_vault_get` and `cui_state` must be a valid C string (NUL-terminated).

**Independent Test**: Documentation in header or source states that `key` must be NUL-terminated.

### Implementation for User Story 4

- [x] T010 [P] [US4] Add API comment for the `key` parameter of `cui_vault_get` and `cui_state` in include/clearui.h or src/core/vault.c stating that `key` must be a valid C string (NUL-terminated)

**Checkpoint**: User Story 4 is done when the vault key contract is documented.

---

## Phase 7: User Story 5 – Use Bounded String Copy in test_vault (Priority: P3)

**Goal**: Replace `strcpy` with a bounded alternative (e.g. `snprintf(str, sizeof(str), "%s", "hello")`) in tests/unit/test_vault.c so the test remains safe if buffer size or source changes.

**Independent Test**: No `strcpy` used for the vault key/buffer test in test_vault.c; build and unit tests pass with unchanged behavior.

### Implementation for User Story 5

- [x] T011 [US5] Replace strcpy with snprintf (or equivalent bounded copy) for the vault key/buffer test in tests/unit/test_vault.c; ensure buffer size is used (e.g. sizeof(dest)) and assertions still pass

**Checkpoint**: User Story 5 is done when test_vault.c uses bounded copy and unit tests pass.

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Verify no regressions and that quickstart guidance holds

- [x] T012 [P] Run make unit-tests and make integration-tests; fix any regressions from Phase 3–7
- [x] T013 Run quickstart.md validation: confirm docs/comment locations match specs/014-security-docs-overflow/quickstart.md and contracts/api-contracts-security.md

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — can start immediately.
- **Phase 2 (Foundational)**: Depends on Phase 1 — review contract doc before editing code.
- **Phases 3–7 (User Stories)**: All depend on Phase 2. User stories have no cross-dependencies; they can be implemented in parallel or in priority order (US1 → US2 → US3 → US4 → US5).
- **Phase 8 (Polish)**: Depends on completion of all user story phases that are being delivered.

### User Story Dependencies

- **US1 (P1)**: No dependency on US2–US5. Can start after Phase 2.
- **US2 (P1)**: No dependency on US1, US3–US5. Can start after Phase 2.
- **US3 (P2)**: No dependency on US1, US2, US4, US5. Can start after Phase 2.
- **US4 (P2)**: No dependency on US1–US3, US5. Can start after Phase 2.
- **US5 (P3)**: No dependency on US1–US4. Can start after Phase 2.

### Within Each User Story

- US1: T003 (doc) can be done first; T004 (optional cap) and T005 (optional test) can follow.
- US2: T006 and T007 are independent (different comment locations).
- US3: T008 and T009 are independent (arena vs frame_alloc).
- US4: T010 only.
- US5: T011 only.

### Parallel Opportunities

- **Phase 2**: T002 is standalone.
- **Phase 3**: T003 and (if implemented) T005 can run in parallel; T004 depends on T003 if doc and implementation are in the same file.
- **Phase 4**: T006 and T007 can run in parallel.
- **Phase 5**: T008 and T009 can run in parallel.
- **Phase 6**: T010 is standalone.
- **Phase 7**: T011 is standalone.
- **Phase 8**: T012 can run in parallel with T013 (run tests vs. validate docs).
- **Across stories**: After Phase 2, US1, US2, US3, US4, and US5 can be worked on in parallel (different files).

---

## Parallel Example: User Story 1

```text
# After Phase 2, implement US1:
Task T003: Add API comment for cui_frame_printf (include/clearui.h or src/core/context.c)
Task T004: (Optional) Enforce max length in cui_frame_printf (src/core/context.c)
Task T005: (Optional) Unit test for length cap (tests/unit/)
```

---

## Parallel Example: User Stories 2–4

```text
# After Phase 2, implement US2, US3, US4 in parallel:
US2: T006 (atlas.c utf8_next comment), T007 (atlas.c cui_font_measure comment)
US3: T008 (arena.c grow), T009 (frame_alloc.c grow)
US4: T010 (vault key comment in clearui.h or vault.c)
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 (Setup) and Phase 2 (Foundational).
2. Complete Phase 3 (US1): T003 (document frame_printf); optionally T004 (length cap) and T005 (test).
3. **Stop and validate**: Confirm documentation is present and, if cap implemented, that oversized format returns "".
4. Optionally deploy or continue with US2–US5.

### Incremental Delivery

1. Phase 1 + 2 → foundation ready.
2. Add US1 (frame_printf) → validate → MVP for “format and allocation” contract.
3. Add US2 (UTF-8 doc) → validate.
4. Add US3 (arena/frame overflow) → run existing tests.
5. Add US4 (vault key doc) → validate.
6. Add US5 (test_vault bounded copy) → run unit tests.
7. Phase 8 (Polish) → full validation.

### Parallel Team Strategy

After Phase 2:

- Developer A: US1 (context.c / clearui.h).
- Developer B: US2 (atlas.c).
- Developer C: US3 (arena.c, frame_alloc.c).
- Developer D: US4 (clearui.h or vault.c) and US5 (test_vault.c).

Stories do not conflict; merge order can be any of P1 → P2 → P3.

---

## Notes

- [P] tasks = different files or independent edits; safe to run in parallel.
- [Story] label maps each task to a user story for traceability.
- Each user story is independently completable and verifiable (doc review and/or existing tests).
- Commit after each task or logical group (e.g. after each story).
- Optional tasks (T004, T005): implement if the feature owner wants the length cap and/or extra test; otherwise skip.
- Avoid: editing the same line in the same file from two tasks; leaving documentation and code out of sync with contracts/api-contracts-security.md.
