# Implementation Plan: 1.0.0 Release

**Branch**: `024-release-1-0-0` | **Date**: 2026-03-08 | **Spec**: [spec.md](spec.md)  
**Input**: 1.0.0 Release Checklist from `ROADMAP.md`

## Summary

Final release verification: run full CI suite (Linux, macOS, Windows), sanitizers (ASan, UBSan), leak-check (LeakSanitizer); verify CHANGELOG covers all milestones; verify README has final API surface; ensure demo works headless and with SDL3; confirm API docs exist; bump version to 1.0.0; tag and release.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None for core; SDL3 optional for demo window; Doxygen optional for docs.  
**Storage**: N/A  
**Testing**: Existing unit (24) and integration (5) tests; ASan, UBSan, LeakSanitizer.  
**Target Platform**: Tier 1 (Ubuntu, macOS, Windows).  
**Project Type**: Library (ClearUI).  
**Performance Goals**: N/A (release verification only).  
**Constraints**: No new features; API frozen since 0.10.0.  
**Scale/Scope**: Version bump + docs + CI verification + tag.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] **Clarity**: No API changes; docs and demo improve clarity.
- [x] **Ownership**: No memory model changes.
- [x] **Zero-to-Window**: Demo with SDL3 validates this; headless path works.
- [x] **C99/C11**: No language changes.
- [x] **Performance**: No hot-path changes.
- [x] **Beautiful & Accessible**: No regression.
- [x] **Tech constraints**: API remains frozen; no new symbols.
- [x] **Layers**: No layer violations.

## Project Structure

### Documentation (this feature)

```text
specs/024-release-1-0-0/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
└── tasks.md
```

### Source Code (repository root)

```text
include/clearui.h          # version bump to 1.0.0
CHANGELOG.md               # add 1.0.0 entry
README.md                  # verify final surface
docs/API.md                # verify current
docs/MIGRATION.md          # verify current
examples/demo.c            # verify runs with SDL3
.github/workflows/ci.yml   # ensure all branches covered
```

## Complexity Tracking

None. This milestone is verification and release; no new features or architectural changes.
