# Implementation Plan: Dynamic Draw Command Buffer or Configurable Capacity

**Branch**: `009-dynamic-draw-buffer` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/009-dynamic-draw-buffer/spec.md`

## Summary

Make the draw command buffer capacity configurable at context creation (e.g. via `cui_config`) so applications can request a larger buffer without recompiling; default remains 1024 for backward compatibility. Optional stretch goal: support dynamic growth when capacity is exceeded. Implementation will extend `cui_config` with a draw-buffer capacity field and change the buffer from a fixed embedded array to a heap-allocated (or configurable-size) buffer owned by the context, with explicit init/teardown to satisfy Constitution (no hidden allocations).

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None (stdlib malloc/free for dynamic buffer; optional static allocation path)
**Storage**: Draw command buffers owned by `cui_ctx`; allocated at create, freed at destroy
**Testing**: Existing unit/integration tests; optional test that custom capacity is honored and overflow returns -1
**Target Platform**: Linux, macOS, Windows (Tier 1)
**Project Type**: Library
**Performance Goals**: 60fps; no extra branch in push hot path beyond capacity check; allocation only at init
**Constraints**: C11 only; no new public API beyond config field(s); RDI still consumes same buffer type
**Scale/Scope**: Core layer (draw_cmd.h/c, context.c), render.c; include/clearui.h for cui_config; ~5 files

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: Capacity set explicitly in `cui_config`; buffer ownership in context (alloc at create, free at destroy). No macro magic or implicit globals.
- [x] **Ownership**: Buffers owned by context; freed in `cui_destroy`. Arena/frame/Vault unchanged; no `free()` on UI nodes.
- [x] **Zero-to-Window**: Build and first-run unchanged. New config field optional (0 or default = 1024).
- [x] **C99/C11**: malloc/free only; no C++ or compiler extensions.
- [x] **Performance**: Single capacity check per push; allocation only at context create. 60fps preserved.
- [x] **Beautiful & Accessible**: No impact on defaults or a11y.
- [x] **Tech constraints**: One new field on `cui_config`; no new functions. API surface within cap.
- [x] **Layers**: Core only (draw_cmd, context); render consumes buffer. Dependencies downward.

## Project Structure

### Documentation (this feature)

```text
specs/009-dynamic-draw-buffer/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 (config + buffer contract)
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
include/
└── clearui.h            # cui_config: add draw_buf_capacity (and optionally dynamic flag)

src/core/
├── draw_cmd.h           # cui_draw_command_buffer: capacity field, cmd pointer or flexible array
├── draw_cmd.c           # push/clear use capacity; init/fini helpers if buffer is heap
├── context.c            # Allocate buffers from config at create; free at destroy
└── render.c             # scale_buf: use buffer capacity/count, not CUI_DRAW_BUF_MAX macro

tests/
└── unit/                # Optional: test custom capacity and overflow return -1
```

**Structure Decision**: Single-project C library. Config in public header; buffer layout in core draw_cmd; context owns allocation. No new layers.

## Complexity Tracking

No constitution violations. Optional dynamic growth (if added) will use explicit realloc in push path, documented; default remains fixed cap.
