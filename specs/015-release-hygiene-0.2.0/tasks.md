# Tasks: Release Hygiene (0.2.0)

**Input**: Design documents from `specs/015-release-hygiene-0.2.0/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Not requested in spec; no test tasks included.

**Organization**: Tasks grouped by user story (US1 = documented contracts + CHANGELOG, US2 = sanitizers + format, US3 = Windows CI) for independent implementation and validation.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `include/`, `src/`, `tests/`, `.github/workflows/` at repository root
- Paths below use repo root as base

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Confirm version and release artifacts exist before documentation work.

- [x] T001 Confirm `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH` in `include/clearui.h`; set MINOR=2 and PATCH=0 when cutting 0.2.0 release (FR-001).

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: CHANGELOG and structure that all release/documentation work depends on.

**Checkpoint**: CHANGELOG exists; user story implementation can begin.

- [x] T002 Create `CHANGELOG.md` at repo root with [Keep a Changelog](https://keepachangelog.com/) structure: header, Unreleased section, and `[0.2.0] - YYYY-MM-DD` placeholder with Added/Changed/Fixed/Deprecated/Removed/Security (FR-002, research.md).

---

## Phase 3: User Story 1 — Maintainer ships 0.2.0 with documented contracts (Priority: P1) — MVP

**Goal**: Maintainer can tag 0.2.0 with CHANGELOG and documented color/threading/limits in header and README; CI includes Windows and sanitizers so the project meets advertised support.

**Independent Test**: Run full CI (Linux, macOS, Windows); open CHANGELOG and `include/clearui.h` and README and verify color format, single-threaded contract, and fixed limits are clearly stated.

### Implementation for User Story 1

- [x] T003 [P] [US1] Document color format `0xAARRGGBB` in `include/clearui.h` and `README.md` per `specs/015-release-hygiene-0.2.0/contracts/public-header.md` (FR-003).
- [x] T004 [P] [US1] Document single-threaded contract (all API calls on one thread) in `include/clearui.h` and `README.md` (FR-004).
- [x] T005 [P] [US1] Document fixed limits and silent-truncation behavior in `include/clearui.h` and `README.md`: `CUI_PARENT_STACK_MAX` (16), `CUI_FOCUSABLE_MAX` (64), `CUI_A11Y_MAX` (128), `CUI_LAST_CLICKED_ID_MAX` (64), `CUI_FRAME_PRINTF_MAX` (65536) per contracts and FR-005.

**Checkpoint**: Header and README contain color, threading, and limits; CHANGELOG exists. US1 doc scope complete.

---

## Phase 4: User Story 2 — Contributor runs sanitizers and format (Priority: P2)

**Goal**: Contributor can run ASan/UBSan builds and format with `.clang-format` to catch memory/undefined issues and match project style.

**Independent Test**: `make asan` and `make ubsan` run unit and integration tests; `clang-format -i` uses repo `.clang-format` and produces no unrelated diff on current tree.

### Implementation for User Story 2

- [x] T006 [P] [US2] Add ASan build and test target(s) to `Makefile` (e.g. `asan`) with `-fsanitize=address`, `-fno-omit-frame-pointer`, `-g`; run unit and integration tests per research.md (FR-007).
- [x] T007 [P] [US2] Add UBSan build and test target(s) to `Makefile` (e.g. `ubsan`) with `-fsanitize=undefined`, `-fno-omit-frame-pointer`, `-g`; run unit and integration tests per research.md (FR-007).
- [x] T008 [P] [US2] Add `.clang-format` at repo root with C-oriented style (e.g. BasedOnStyle: LLVM or Linux, 4-space indent, column limit) per research.md (FR-008).
- [x] T009 [US2] Document sanitizer targets (`make asan`, `make ubsan`) and `clang-format -i` usage in `README.md` or CONTRIBUTING (FR-008).

**Checkpoint**: Contributors can run sanitizers and format; US2 complete.

---

## Phase 5: User Story 3 — CI includes Windows (MSVC) (Priority: P2)

**Goal**: GitHub Actions matrix includes Windows (MSVC) and at least one sanitizer run so README’s MSVC claim is validated on every push.

**Independent Test**: Push a change and confirm Windows job and sanitizer step run and succeed.

### Implementation for User Story 3

- [x] T010 [US3] Add Windows (MSVC) job to `.github/workflows/ci.yml` that builds (e.g. `make all` or equivalent with MSVC) and runs unit and integration tests (FR-006, research.md).
- [x] T011 [US3] Add CI step(s) in `.github/workflows/ci.yml` to run ASan and/or UBSan build and tests (FR-007).

**Checkpoint**: CI runs on Ubuntu, macOS, and Windows; sanitizers run in CI. US3 complete.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Final doc and validation pass.

- [x] T012 [P] Update `README.md` CI section to mention Windows (MSVC) and sanitizer runs (and link to `ci.yml`).
- [x] T013 Populate `CHANGELOG.md` `[0.2.0]` entry with Added items for this milestone (CHANGELOG, color/threading/limits docs, Windows CI, ASan/UBSan, `.clang-format`).
- [x] T014 Run validation from `specs/015-release-hygiene-0.2.0/quickstart.md` (build, sanitizers, format, CHANGELOG steps).

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately.
- **Phase 2 (Foundational)**: Depends on Phase 1 — CHANGELOG creation.
- **Phase 3 (US1)**: Depends on Phase 2 — documentation in header and README.
- **Phase 4 (US2)**: Depends on Phase 2 — Makefile and `.clang-format`; can run in parallel with Phase 5 after Phase 3.
- **Phase 5 (US3)**: Depends on Phase 2 — CI changes; can run in parallel with Phase 4 after Phase 3.
- **Phase 6 (Polish)**: Depends on Phases 3–5 — final README, CHANGELOG text, quickstart validation.

### User Story Dependencies

- **US1 (P1)**: After Phase 2. Delivers: CHANGELOG, color/threading/limits in header and README.
- **US2 (P2)**: After Phase 2. Delivers: `make asan`, `make ubsan`, `.clang-format`, contributor docs. Independent of US3.
- **US3 (P2)**: After Phase 2. Delivers: Windows job and sanitizer step in CI. Independent of US2.

### Parallel Opportunities

- **Phase 3**: T003, T004, T005 can run in parallel (different sections of same files or separate edits).
- **Phase 4**: T006, T007, T008 can run in parallel (Makefile vs `.clang-format` vs doc).
- **Phase 5**: T010 and T011 can be done in one or two edits to `ci.yml`.
- **Phase 6**: T012 (README) is [P] with T013 (CHANGELOG).

---

## Parallel Example: User Story 1

```bash
# Document all three contract areas (coordinate if editing same files):
Task T003: Document color format in include/clearui.h and README.md
Task T004: Document single-threaded contract in include/clearui.h and README.md
Task T005: Document fixed limits in include/clearui.h and README.md
```

---

## Implementation Strategy

### MVP First (User Story 1)

1. Phase 1: T001 (version macros).
2. Phase 2: T002 (CHANGELOG).
3. Phase 3: T003–T005 (color, threading, limits in header + README).
4. **Validate**: Read header and README; confirm CHANGELOG structure.
5. Optionally add 0.2.0 entry and tag when ready.

### Incremental Delivery

1. **MVP (US1)**: Phases 1–3 → documented contracts and CHANGELOG.
2. **US2**: Phase 4 → contributors can run sanitizers and format.
3. **US3**: Phase 5 → CI includes Windows and sanitizers.
4. **Polish**: Phase 6 → README CI section, CHANGELOG text, quickstart check.

### Parallel Team Strategy

- After Phase 2: one person can do US1 (T003–T005), another US2 (T006–T009), another US3 (T010–T011).
- Phase 6 can follow once US1–US3 are done.

---

## Notes

- [P] = different files or independent edits; coordinate if touching same file.
- No test tasks; spec does not require new tests.
- Version macros: set MINOR=2, PATCH=0 in `include/clearui.h` when cutting 0.2.0 (T001 or at release).
- Limits are already defined in `src/core/context.c` and `src/core/a11y.h`; T005 only adds documentation in header and README.
