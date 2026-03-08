# Implementation Plan: Theming (0.6.0)

**Branch**: `019-theming-0.6.0` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `/specs/019-theming-0.6.0/spec.md`

## Summary

Replace compile-time theme `#define`s in `src/core/theme.h` with a runtime `cui_theme` struct and `cui_set_theme(ctx, &theme)`. Context holds the current theme (copy); when NULL or unset, use a built-in default. Update `draw_cmd.c` and `layout.c` to read theme from context. Ship a dark theme preset. Add a test that applies a theme and asserts draw command colors change. No new allocations in hot path; theme is a fixed struct stored in context.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None (stdlib only)  
**Storage**: Theme stored in `cui_ctx` (struct copy or pointer to default); app-owned theme struct when set  
**Testing**: Makefile unit tests; new test applies theme and asserts draw buffer colors  
**Target Platform**: Same as ClearUI (Windows, macOS, Linux)  
**Project Type**: Library (ClearUI)  
**Performance Goals**: 60fps; theme lookup is a struct read, no indirection cost  
**Constraints**: No new public API beyond `cui_set_theme` and `cui_theme`; preserve WCAG 2.1 AA focus ring semantics  
**Scale/Scope**: One active theme per context; single preset (dark) in 0.6.0

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: Theme struct and setter are explicit; no hidden state beyond “current theme” in context.
- [x] **Ownership**: Theme is copied into context (or context holds pointer to internal default); app does not free theme; no `free()` on UI nodes.
- [x] **Zero-to-Window**: Build and first-run unchanged; default theme preserves current look.
- [x] **C99/C11**: C11 only; theme struct is plain data.
- [x] **Performance**: Single struct in context; no extra allocations in draw path.
- [x] **Beautiful & Accessible**: Default theme unchanged; dark preset and focus ring remain; a11y not degraded.
- [x] **Tech constraints**: One new public type + one setter; API surface well under cap.
- [x] **Layers**: Theme lives in core; draw_cmd and layout read from context; no layer violation.

## Project Structure

### Documentation (this feature)

```text
specs/019-theming-0.6.0/
├── plan.md              # This file
├── spec.md              # Feature spec (created)
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (theme API contract)
└── tasks.md             # Phase 2 output (/speckit.tasks - not by plan)
```

### Source Code (repository root)

```text
include/
├── clearui.h            # cui_theme struct, cui_set_theme declaration; optional dark preset decl

src/
├── core/
│   ├── context.c        # theme field in cui_ctx; init to default; cui_set_theme copies or resets
│   ├── theme.h          # keep for default values / internal use; or replace with default struct
│   └── ...
├── layout/
│   └── layout.c         # read font size (and any theme layout) from context theme
└── ...
  (draw_cmd.c uses theme from context via accessor or ctx)

tests/
└── unit/
    └── test_theme.c     # set theme, build frame, assert draw buffer colors
```

**Structure Decision**: Single project; theme is a core concern. Public `cui_theme` and `cui_set_theme` in `clearui.h`; internal default in core; `draw_cmd.c` and `layout.c` take context and read theme from it (e.g. `cui_ctx_theme(ctx)` or direct ctx field if in same layer).

## Complexity Tracking

> No constitution violations. This section left empty.
