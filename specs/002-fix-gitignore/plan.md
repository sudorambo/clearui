# Implementation Plan: Fix .gitignore and Remove Tracked Build Artifacts

**Branch**: `002-fix-gitignore` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/002-fix-gitignore/spec.md`

## Summary

The Makefile generates test binaries (`test_arena`, `test_counter`, `test_hello`, `test_layout`, `test_rdi_platform`, `test_vault`) in the repo root. These 6 binaries are currently tracked by git. The `.gitignore` already covers `*.o` but lacks rules for test binaries. This plan adds the missing ignore rules and removes tracked binaries from the index.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None (zero external dependencies)
**Storage**: N/A
**Testing**: `make unit-tests && make integration-tests` (assert-based C tests)
**Target Platform**: Linux (Ubuntu), macOS (CI matrix)
**Project Type**: Library (C GUI library)
**Performance Goals**: N/A (build hygiene fix)
**Constraints**: Must not break CI; must not delete files from disk
**Scale/Scope**: 1 file changed (`.gitignore`), 6 files untracked from index

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: No code changes; `.gitignore` is self-documenting. N/A for API clarity.
- [x] **Ownership**: No memory model changes. N/A.
- [x] **Zero-to-Window**: Build and first-run remain single-command. No change to build process.
- [x] **C99/C11**: No source code changes. N/A.
- [x] **Performance**: No runtime changes. N/A.
- [x] **Beautiful & Accessible**: No UI or a11y changes. N/A.
- [x] **Tech constraints**: No API surface changes. N/A.
- [x] **Layers**: No architecture changes. N/A.

**Result**: All gates pass. This is a build-hygiene-only change with no impact on the library's code, API, or architecture.

## Project Structure

### Documentation (this feature)

```text
specs/002-fix-gitignore/
├── plan.md              # This file
├── research.md          # Phase 0: investigation of tracked artifacts
├── data-model.md        # Phase 1: N/A (no data entities)
└── quickstart.md        # Phase 1: verification steps
```

### Source Code (repository root)

```text
.gitignore               # Updated: add test binary ignore rules
```

No source files are modified. Only `.gitignore` and the git index are affected.

## Complexity Tracking

No constitution violations. Table not needed.
