# Implementation Plan: Review Fixes

**Branch**: `012-review-fixes` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/012-review-fixes/spec.md`

## Summary

Fix bugs, correctness issues, API design problems, and test coverage gaps identified in a deep repository review. The changes span the draw command API (`cui_draw_circle` draws a square), allocation failure handling (NULL pushed to parent stack), type safety (`void *` in public API), accessibility state composition, `cui_text_input_opts` type error, hit-testing gap for text inputs, arena alignment validation, enum consistency, test coverage expansion, and build system improvements.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None (vendored `stb_truetype.h` for font metrics)  
**Storage**: N/A  
**Testing**: Plain C with `assert.h`, Makefile targets (`make unit-tests`, `make integration-tests`)  
**Target Platform**: Linux, macOS (CI on `ubuntu-latest`, `macos-latest`)  
**Project Type**: Library (C11 declarative immediate-mode UI)  
**Performance Goals**: 60fps on integrated GPUs for typical UIs  
**Constraints**: Zero external dependencies for default build; API surface ~60-120 functions  
**Scale/Scope**: ~3,500 LOC across 35 source files; 9 test files

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: All changes improve clarity — typed API params replace `void *`, `cui_draw_circle` will match its name, alignment validation prevents silent UB.
- [x] **Ownership**: Memory model unchanged. Fix prevents NULL corruption in parent stack (arena allocation failure). No new `free()` on UI nodes.
- [x] **Zero-to-Window**: No build system changes that affect first-run. `libclearui.a` target is additive.
- [x] **C99/C11**: All changes are pure C11. No new dependencies.
- [x] **Performance**: No performance impact. Rounded rect push function adds one more command type to an existing code path. Alignment validation is a single branch.
- [x] **Beautiful & Accessible**: A11y state composition fix directly improves accessibility. Hit-testing text inputs improves keyboard/mouse parity.
- [x] **Tech constraints**: RDI/platform abstraction unchanged. API surface: adding `cui_draw_buf_push_rounded_rect` (~1 function); well within cap.
- [x] **Layers**: All changes respect four-layer architecture. No upward dependencies introduced.

## Project Structure

### Documentation (this feature)

```text
specs/012-review-fixes/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output (N/A for bug fixes)
├── contracts/           # Phase 1 output
└── tasks.md             # Phase 2 output
```

### Source Code (repository root)

```text
src/
├── core/
│   ├── a11y.c           # Fix: compose checked+focused state
│   ├── arena.c          # Fix: alignment validation
│   ├── context.c        # Fix: hit-test text inputs
│   ├── draw_cmd.c       # Add: push_rounded_rect; remove unused macro
│   ├── draw_cmd.h       # Add: push_rounded_rect declaration; remove CUI_DRAW_BUF_MAX
│   ├── node.h           # Fix: typedef enum consistency
│   └── render.c         # Fix: handle rounded rect in scaling
├── widget/
│   ├── canvas.c         # Fix: cui_draw_circle emits rounded rect
│   └── layout.c         # Fix: NULL guard on allocation failure
├── layout/
│   └── (no changes)
├── font/
│   └── (no changes)
├── platform/
│   └── (no changes)
└── rdi/
    └── (no changes)

include/
├── clearui.h            # Fix: typed params for set_platform/set_rdi; fix text_input_opts
├── clearui_platform.h   # (no changes)
└── clearui_rdi.h        # (no changes)

tests/
├── unit/
│   ├── test_arena.c     # Fix: add PASS print; test alignment validation
│   ├── test_vault.c     # Fix: add PASS print
│   ├── test_a11y.c      # NEW: accessibility tree tests
│   ├── test_frame_alloc.c # NEW: frame allocator tests
│   ├── test_draw_cmd.c  # NEW: draw command push/clear tests
│   └── test_focus.c     # NEW: keyboard focus/tab tests
└── integration/
    └── (existing tests unchanged)
```

**Structure Decision**: Single project, existing `src/` + `tests/` layout. No structural changes needed.

## Complexity Tracking

No constitution violations. All changes are bug fixes, correctness improvements, or additive test coverage within the existing architecture.
