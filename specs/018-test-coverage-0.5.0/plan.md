# Implementation Plan: Test Coverage (0.5.0)

**Branch**: `018-test-coverage-0.5.0` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `/specs/018-test-coverage-0.5.0/spec.md`

## Summary

Add test coverage for public APIs that are currently untested or under-tested: canvas and draw commands (draw buffer contents), `cui_label_styled`, `cui_spacer`, `cui_wrap`, `cui_stack`, style stack push/pop, frame allocator lifetime, Hi-DPI scale_buf path, and edge cases (NULL, zero-size, empty trees). No new runtime API; existing Makefile targets (`asan`, `ubsan`) continue to run the full test suite including new tests. Target: every public API function listed in the milestone exercised by at least one test.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None (stdlib only; existing stub platform/RDI for tests)  
**Storage**: N/A (tests only; no new persistent state)  
**Testing**: Makefile-driven unit and integration tests (existing pattern); new tests in `tests/unit/` and `tests/integration/` as needed  
**Target Platform**: Same as ClearUI (Linux, macOS, Windows); tests run in CI  
**Project Type**: Library (ClearUI)  
**Performance Goals**: Tests complete in reasonable time; no performance regression in library code  
**Constraints**: Tests must not require graphics output; use stub platform and software RDI; assert on draw buffer contents or layout/style state  
**Scale/Scope**: One or more test files per area (canvas, layout widgets, style, frame allocator, Hi-DPI, edge cases); total new tests on the order of a dozen or more test cases

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: No hidden allocations, macro magic, or implicit global state; API readable as pseudocode. (Tests do not change public API.)
- [x] **Ownership**: Memory model (arena / frame allocator / Vault) respected; no `free()` on UI nodes. (Tests exercise existing behavior; edge-case tests may pass NULL.)
- [x] **Zero-to-Window**: Build and first-run remain single-command / under 5 min where applicable. (New tests are additive; `make unit-tests` / `make integration-tests` unchanged in workflow.)
- [x] **C99/C11**: No C++, no compiler extensions; optional deps documented. (Tests are C11.)
- [x] **Performance**: 60fps target, GPU-friendly pipeline; layout cost acceptable. (Tests do not add runtime cost to library.)
- [x] **Beautiful & Accessible**: Defaults and a11y not degraded. (Coverage only.)
- [x] **Tech constraints**: RDI/platform abstraction and API surface cap (~120 functions) respected. (No new public API.)
- [x] **Layers**: Four-layer architecture; dependencies flow downward only. (Tests link library and call public or test-only accessors as needed.)

## Project Structure

### Documentation (this feature)

```text
specs/018-test-coverage-0.5.0/
├── plan.md              # This file
├── spec.md              # Feature spec (created)
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output (minimal; test artifacts only)
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (test contracts for APIs under test)
└── tasks.md             # Phase 2 output (/speckit.tasks - not by plan)
```

### Source Code (repository root)

```text
include/
├── clearui.h            # No change (no new API)
└── clearui_platform.h   # No change

src/
├── core/                # No structural change; tests may rely on existing context/node accessors
├── layout/
├── widget/
└── ...

tests/
├── unit/
│   ├── test_canvas_draw.c    # Canvas + draw_rect/circle/text buffer contents
│   ├── test_label_styled.c   # cui_label_styled style application
│   ├── test_spacer.c         # cui_spacer layout sizing
│   ├── test_wrap.c           # cui_wrap flow wrapping
│   ├── test_stack.c          # cui_stack z-order overlap
│   ├── test_style_stack.c    # push_style / pop_style nesting and restoration
│   ├── test_frame_alloc.c    # frame allocator lifetime (reset after begin_frame)
│   ├── test_scale_buf.c      # Hi-DPI scale_buf path at scale_factor > 1
│   └── test_edge_cases.c     # NULL args, zero-size, empty tree
└── integration/
    └── (optional)           # If any scenario needs full frame loop
```

**Structure Decision**: Single project; all new artifacts are test files under `tests/unit/` (and optionally `tests/integration/`). Makefile gains new test targets and adds them to `unit-tests` / `integration-tests` and `clean`. Existing `make asan` and `make ubsan` (0.2.0) run the full suite including new tests.

## Complexity Tracking

> No constitution violations. This section left empty.
