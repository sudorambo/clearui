# Tasks: API Polish (0.10.0)

**Input**: Design documents from `specs/023-api-polish-0-10-0/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Tasks grouped by user story; dependencies ordered so setup and independent items first, then API review, then docs and freeze.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files or no shared dependencies)
- **[Story]**: US1–US6 from spec.md

---

## Phase 1: Version string & layout docs (no dependency on API review)

- [x] **T001** [P] [US4] Add `const char *cui_version_string(void);` declaration to `include/clearui.h` (near version macros or with other query APIs). Document that it returns `"MAJOR.MINOR.PATCH"` and is not thread-safe.
- [x] **T002** [US4] Implement `cui_version_string()` in `src/core/` (new `version.c` or existing file): return string from `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH` (e.g. static buffer + snprintf, or literal). Add `version.c` to Makefile if new; ensure no allocation.
- [x] **T003** [P] [US2] In `include/clearui.h`, expand the `cui_layout` comment to state: "If `padding_x` or `padding_y` is greater than 0, they override `padding` for that axis; otherwise `padding` sets both. Zero-init `(cui_layout){}` = no padding." Update README layout/theming section if present to mention the same precedence.
- [x] **T004** [US2] Verify layout code (e.g. `src/layout/` or wherever padding is read) resolves padding per contract: effective_x = padding_x > 0 ? padding_x : padding; effective_y = padding_y > 0 ? padding_y : padding. Fix if inconsistent.

---

## Phase 2: API review (US1)

- [x] **T005** [US1] Audit every public function in `include/clearui.h`, `include/clearui_platform.h`, `include/clearui_rdi.h`: ensure `cui_ctx *ctx` (or equivalent) first, optional config/opts as `const X *`, consistent return conventions (int for success/click, pointer for getters). Produce a short audit list (e.g. in spec or comment); fix only non-breaking inconsistencies (naming, parameter order, comments).

---

## Phase 3: Optional widgets (US3)

- [x] **T006** [US3] Evaluate adding `cui_image` and/or `cui_tooltip`/`cui_popup` per research and API cap (~120 functions). If implementing: add declarations in `include/clearui.h`, implement in `src/` following existing widget patterns (ctx first, retained state, a11y if interactive). If not: add "Reserved for 1.x" note in `specs/023-api-polish-0-10-0/spec.md` and leave API unchanged.

---

## Phase 4: API reference & migration guide (US5)

- [x] **T007** [US5] Add Doxygen comments (brief + params) to public APIs in `include/clearui.h`, `include/clearui_platform.h`, `include/clearui_rdi.h`. Add minimal `Doxyfile` and a Makefile target (e.g. `make docs`) to generate output into `docs/api/`. Alternatively: add a single Markdown file (e.g. `docs/API.md`) listing all public functions and types with one-line descriptions.
- [x] **T008** [US5] Add `docs/MIGRATION.md` (or a **Migration** section in README): one subsection per version from 0.2.0 to 0.10.0, listing breaking or notable API changes and short upgrade notes (e.g. poll_events, layout_run(ctx), theme, error callback, version string). Use CHANGELOG and git history as sources.

---

## Phase 5: Freeze & polish (US6)

- [x] **T009** [US6] In README.md or CONTRIBUTING.md, state that the public API is frozen after 0.10.0 and that there will be no breaking changes until 1.0; new APIs must be additive only.
- [x] **T010** Update `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH` in `include/clearui.h` to `0`, `10`, `0` for the 0.10.0 release. Ensure `cui_version_string()` output stays in sync.
- [x] **T011** Run `make unit-tests` (and any other project tests); fix any regressions from API or layout documentation changes.

---

## Dependencies & execution order

| Phase | Tasks    | Depends on      |
|-------|----------|-----------------|
| 1     | T001–T004| None            |
| 2     | T005     | None (can run in parallel with Phase 1) |
| 3     | T006     | T005 (API review done so cap is known)  |
| 4     | T007–T008| T005 (stable API for docs)             |
| 5     | T009–T011| T001–T008       |

**Suggested order**: T001 → T002; T003 → T004; T005; then T006, T007, T008 (T007/T008 can be parallel); then T009, T010, T011.

---

## Checkpoints

- **After Phase 1**: Version string and layout padding semantics are implemented and documented; layout code matches contract.
- **After Phase 2**: Public API is reviewed and consistent; audit list exists.
- **After Phase 3**: Optional widgets either implemented or documented as reserved for 1.x.
- **After Phase 4**: API reference and migration guide exist.
- **After Phase 5**: Freeze is documented; version set to 0.10.0; all tests pass.
