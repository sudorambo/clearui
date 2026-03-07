# Feature Spec: Dynamic Draw Command Buffer or Configurable Capacity

**Branch**: `009-dynamic-draw-buffer` | **Date**: 2026-03-07

## Problem

The draw command buffer is a fixed-size array: `cui_draw_command_buffer` in `src/core/draw_cmd.h` embeds `cui_draw_cmd cmd[CUI_DRAW_BUF_MAX]` with `CUI_DRAW_BUF_MAX 1024`. The context (`src/core/context.c`) holds two such buffers (`draw_buf`, `canvas_cmd_buf`) as struct members. When the UI produces more than 1024 draw commands in a frame, push functions return -1 and commands are dropped silently. The capacity is not configurable, so applications with dense UIs or many canvas primitives cannot adapt without forking the library.

## Goals

1. **Configurable capacity (minimum)**: Allow the application to set the draw buffer capacity (e.g. via `cui_config` or an init option) so that heavy UIs can request a larger buffer without recompiling.
2. **Dynamic buffer (stretch)**: Optionally make the buffer grow on demand (e.g. realloc when full) so that overflow is avoided without a fixed cap, while preserving the option to keep a fixed cap for embedded/safety-critical use.
3. **Constitution compliance**: No hidden allocations unless explicitly opted in; explicit ownership; C11-only; performance by design (no unnecessary realloc in hot path if avoidable).

## Requirements

1. **R1**: Draw buffer capacity MUST be configurable at context creation (e.g. `cui_config.draw_buf_capacity` or similar). Default remains 1024 when not set.
2. **R2**: When capacity is exceeded, behavior MUST be defined: either fail push (return -1, current behavior) or grow the buffer if dynamic mode is enabled. Callers MUST be able to detect failure when not growing.
3. **R3**: Public API surface MUST remain within constitution limits (~60–80 functions). Prefer extending `cui_config` over adding new init functions.
3. **R4**: All existing tests MUST pass. No mandatory new dependencies.

## Scope

- **In scope**: `cui_config` (or equivalent), `cui_draw_command_buffer` representation (fixed array vs pointer + capacity), context init and teardown, draw_cmd push/clear, render path (`scale_buf` and RDI submit). Optional: dynamic growth policy and `cui_draw_buf_*` API for capacity/growth.
- **Out of scope**: Changing the RDI contract (still consumes `cui_draw_command_buffer`); changing command types or layout.

## Acceptance Criteria

- Application can set draw buffer capacity at create time (e.g. 2048 or 4096) and get that capacity.
- Default capacity with no config change is 1024 (backward compatible).
- When capacity is exceeded and growth is not enabled, push returns -1 and no silent drop of commands without return-value handling.
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
