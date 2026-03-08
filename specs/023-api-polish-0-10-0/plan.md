# Implementation Plan: API Polish (0.10.0)

**Branch**: `023-api-polish-0-10-0` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/023-api-polish-0-10-0/spec.md`

## Summary

Final API review before 1.0: audit all public function signatures for consistency (ctx first, opts); resolve `cui_layout.padding` vs `padding_x`/`padding_y` (document or deprecate); consider `cui_image`, `cui_tooltip`/`cui_popup` within API cap; add `cui_version_string()`; generate API reference (Doxygen or custom); write migration guide; freeze public API. Technical approach: systematic pass over `include/clearui.h` and related headers; document layout padding semantics; add version string from existing macros; add Doxygen comments or script-generated reference; single MIGRATION.md or README section.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None for core; optional Doxygen for API reference generation.  
**Storage**: N/A (version string is static or derived from macros).  
**Testing**: Existing unit/integration tests; no new mandatory test types.  
**Target Platform**: Tier 1 (Windows, macOS, Linux); docs are cross-platform.  
**Project Type**: Library (ClearUI); public API in `include/clearui.h`, `clearui_platform.h`, `clearui_rdi.h`.  
**Performance Goals**: No impact; version string and docs are one-time or build-time.  
**Constraints**: API surface cap ~120 functions; no breaking changes after freeze.  
**Scale/Scope**: ~60–80 public functions today; image/tooltip/popup optional.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Clarity**: API polish and docs improve clarity; no new hidden behavior.
- [x] **Ownership**: Memory model unchanged; new widgets (if any) follow same model.
- [x] **Zero-to-Window**: Unchanged; version string and docs do not affect first-run.
- [x] **C99/C11**: No new language features; Doxygen is documentation only.
- [x] **Performance**: No hot-path change from polish or version string.
- [x] **Beautiful & Accessible**: No regression; new widgets (if added) follow a11y.
- [x] **Tech constraints**: API cap respected; new symbols justified.
- [x] **Layers**: No layer violation; changes are API-surface and docs.

## Project Structure

### Documentation (this feature)

```text
specs/023-api-polish-0-10-0/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
└── tasks.md             # from /speckit.tasks
```

### Source Code (repository root)

```text
include/
  clearui.h              # public API; layout padding doc/deprecation; version string decl
  clearui_platform.h
  clearui_rdi.h
src/
  core/                  # version string implementation if not header-only
  widget/               # optional: image, tooltip/popup
docs/                    # optional: api/ for generated reference, MIGRATION.md
```

**Structure Decision**: Single project; changes in `include/` and optionally `src/`; docs in repo root or `docs/`.

## Complexity Tracking

None. All items are additive (version string, docs, optional widgets) or clarification (layout padding).
