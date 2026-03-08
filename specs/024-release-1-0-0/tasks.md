# Tasks: 1.0.0 Release

**Input**: Design documents from `specs/024-release-1-0-0/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Sequential release checklist — verify, fix, bump, tag.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: US1–US4 from spec.md

---

## Phase 1: Local verification (US1)

- [x] **T001** [US1] Run `make clean && make all && make unit-tests && make integration-tests` locally; all 24 unit tests and 5 integration tests must pass. Report any failures.
- [x] **T002** [P] [US1] Run `make demo && ./demo` (headless); must exit 0.
- [x] **T003** [US1] Run `make asan` (ASan); must pass with no errors.
- [x] **T004** [P] [US1] Run `make ubsan` (UBSan); must pass with no errors.
- [x] **T005** [US1] Run `make leak-check-lsan` (LeakSanitizer); must pass with no leaks.

---

## Phase 2: Documentation verification (US2)

- [x] **T006** [P] [US2] Verify `CHANGELOG.md` has entries for 0.2.0, 0.3.0, 0.4.0, 0.5.0, 0.6.0, 0.7.0, 0.8.0, 0.9.0, 0.10.0. List any missing versions.
- [x] **T007** [P] [US2] Verify `README.md`: API surface count (~60 functions), docs links (`docs/API.md`, `docs/MIGRATION.md`), freeze statement, project structure. Fix inaccuracies.
- [x] **T008** [P] [US2] Verify `docs/API.md` lists all current public functions from `include/clearui.h`, `include/clearui_platform.h`, `include/clearui_rdi.h`. Fix any missing or outdated entries.
- [x] **T009** [P] [US2] Verify `docs/MIGRATION.md` covers 0.2.0–0.10.0 and has correct upgrade notes. Fix any gaps.

---

## Phase 3: Demo verification (US3)

- [x] **T010** [US3] Verify `examples/demo.c` compiles and runs headlessly (stub platform + software RDI); exercises all public APIs. Note: visible-window test with SDL3 (`make demo WITH_SDL3=1`) requires manual verification — document as checked or skipped.

---

## Phase 4: Version bump & CHANGELOG (US4)

- [x] **T011** [US4] Set `CUI_VERSION_MAJOR=1`, `CUI_VERSION_MINOR=0`, `CUI_VERSION_PATCH=0` in `include/clearui.h`. Verify `cui_version_string()` returns `"1.0.0"` (compile and call or inspect logic).
- [x] **T012** [US4] Add `## [1.0.0]` entry to `CHANGELOG.md` documenting the stable release (version bump, API frozen, all milestones complete).
- [x] **T013** [US4] Update `docs/MIGRATION.md` to add a 1.0.0 section noting the stable release and API freeze.

---

## Phase 5: CI & release (US4)

- [x] **T014** [US4] Ensure `.github/workflows/ci.yml` triggers on the release branch or main. Update `on.push.branches` / `on.pull_request.branches` if needed.
- [x] **T015** [US4] Run `make unit-tests` one final time after version bump to confirm no regressions.
- [x] **T016** [US4] Tag and release: document the commands to run (`git tag v1.0.0`, `gh release create`). Note: actual tag/push requires user confirmation and is a manual step.

---

## Dependencies & execution order

| Phase | Tasks    | Depends on      |
|-------|----------|-----------------|
| 1     | T001–T005| None            |
| 2     | T006–T009| None (can run in parallel with Phase 1) |
| 3     | T010     | T001 (build must pass) |
| 4     | T011–T013| T001–T010 (verification done) |
| 5     | T014–T016| T011–T013 (version bumped) |

**Suggested order**: T001–T005 and T006–T009 in parallel; then T010; then T011→T012→T013; then T014→T015→T016.

---

## Checkpoints

- **After Phase 1**: All tests, sanitizers, and leak-check pass locally.
- **After Phase 2**: All docs verified and up to date.
- **After Phase 3**: Demo works headlessly.
- **After Phase 4**: Version is 1.0.0; CHANGELOG and MIGRATION updated.
- **After Phase 5**: CI passes; tag and release ready.
