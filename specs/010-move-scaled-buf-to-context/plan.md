# Implementation Plan: Move the static scaled_buf in render.c to the context struct

**Branch**: `010-move-scaled-buf-to-context` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/010-move-scaled-buf-to-context/spec.md`

## Summary

Remove the file-static scaled draw command buffer from `render.c` and store it on the context struct instead. The scaled buffer is initialized and finalized with the main draw buffer (same capacity), and the render path uses a context getter (e.g. `cui_ctx_scaled_buf(ctx)`) when scale != 1.f. This satisfies Constitution (no implicit global state; explicit ownership).

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None
**Storage**: Draw buffers owned by `cui_ctx`; scaled_buf is one of them
**Testing**: Existing unit and integration tests; no new tests required
**Target Platform**: Linux, macOS, Windows (Tier 1)
**Project Type**: Library
**Performance Goals**: 60fps; no extra allocation in submit path (buffer pre-allocated at create)
**Constraints**: C11 only; no new public API
**Scale/Scope**: Core only (context.c, context.h, render.c); ~3 files

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: Scaled buffer is explicit context state; no file-static in render.c.
- [x] **Ownership**: Buffer owned by context; init in create, fini in destroy.
- [x] **Zero-to-Window**: No change to build or first-run.
- [x] **C99/C11**: No new language features.
- [x] **Performance**: No alloc in hot path; buffer reused per frame.
- [x] **Beautiful & Accessible**: No impact.
- [x] **Tech constraints**: Optional internal getter only; API cap unchanged.
- [x] **Layers**: Core only; render uses context. Dependencies downward.

## Project Structure

### Documentation (this feature)

```text
specs/010-move-scaled-buf-to-context/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
src/core/
├── context.c            # Add scaled_buf to struct; init/fini in create/destroy; getter cui_ctx_scaled_buf
├── context.h            # Declare cui_ctx_scaled_buf
└── render.c             # Remove static scaled_buf; use cui_ctx_scaled_buf(ctx) when scale != 1.f
```

**Structure Decision**: Single-project C library. Change is confined to Core (context + render). No new layers.

## Complexity Tracking

No constitution violations.
