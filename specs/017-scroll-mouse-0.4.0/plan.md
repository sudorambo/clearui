# Implementation Plan: Scroll & Mouse (0.4.0)

**Branch**: `017-scroll-mouse-0.4.0` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `/specs/017-scroll-mouse-0.4.0/spec.md`

## Summary

Add scroll wheel input (`cui_inject_scroll(ctx, dx, dy)`) wired to `CUI_NODE_SCROLL` offset with clamping to content bounds, and mouse hover tracking (`cui_inject_mouse_move(ctx, x, y)`) with hover state exposed to widgets (e.g. `cui_ctx_hovered_id` or per-id query). Optional: document or use `cui_platform.cursor_set` for cursor shape. Unit tests for scroll clamping; integration test for scroll + visible region.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None (stdlib only; platform adapter for optional cursor_set)  
**Storage**: N/A (scroll offset already in retained node; hover id in context)  
**Testing**: Makefile unit/integration tests (existing pattern); new tests for scroll and hover  
**Target Platform**: Same as ClearUI (Windows, macOS, Linux); scroll/hover are input only  
**Project Type**: Library (ClearUI)  
**Performance Goals**: 60fps; one extra hit-test pass for hover/scroll target is acceptable  
**Constraints**: No new allocations in hot path; scroll/hover state in existing context struct  
**Scale/Scope**: Single scroll target per frame; one hovered widget id (last-hit wins)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: No hidden allocations, macro magic, or implicit global state; API readable as pseudocode.
- [x] **Ownership**: Memory model (arena / frame allocator / Vault) respected; no `free()` on UI nodes.
- [x] **Zero-to-Window**: Build and first-run remain single-command / under 5 min where applicable.
- [x] **C99/C11**: No C++, no compiler extensions; optional deps documented.
- [x] **Performance**: 60fps target, GPU-friendly pipeline; one additional hit-test for hover/scroll is acceptable.
- [x] **Beautiful & Accessible**: Defaults and a11y not degraded; hover improves discoverability.
- [x] **Tech constraints**: RDI/platform abstraction and API surface cap (~120 functions) respected.
- [x] **Layers**: Four-layer architecture; dependencies flow downward only.

## Project Structure

### Documentation (this feature)

```text
specs/017-scroll-mouse-0.4.0/
├── plan.md              # This file
├── spec.md              # Feature spec (created)
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (scroll + hover API)
└── tasks.md             # Phase 2 output (/speckit.tasks - not by plan)
```

### Source Code (repository root)

```text
include/
├── clearui.h            # cui_inject_scroll, cui_inject_mouse_move; hover query API
└── clearui_platform.h   # cursor_set already present

src/
├── core/
│   ├── context.c        # pending_scroll_dx/dy, mouse_x/y, hovered_id; apply scroll; hover hit-test
│   └── node.h           # no change (scroll already has scroll_offset_y, scroll_max_h)
├── widget/
│   └── scroll.c         # optional: add id to scroll for scroll target lookup
layout/
└── layout.c             # content height for scroll (already computed for SCROLL)

tests/
├── unit/
│   └── test_scroll.c    # scroll clamping
└── integration/
    └── test_scroll_region.c  # scroll + verify offset/visible region
```

**Structure Decision**: Single project; changes in `include/`, `src/core/`, `src/widget/`, `tests/` as above.

## Complexity Tracking

None; no constitution violations.
