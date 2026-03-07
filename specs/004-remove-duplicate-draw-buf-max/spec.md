# Feature Spec: Remove Duplicate CUI_DRAW_BUF_MAX in render.c

**Branch**: `004-remove-duplicate-draw-buf-max` | **Date**: 2026-03-07

## Problem

`CUI_DRAW_BUF_MAX` is defined as 1024 in two locations:

1. `src/core/draw_cmd.h:50` — canonical definition, used by `cui_draw_command_buffer` struct
2. `src/core/render.c:10` — duplicate, used by `scale_buf` loop guard

`render.c` already includes `draw_cmd.h` (line 6), so the duplicate is redundant. If the values ever diverge, `scale_buf` could silently overflow or truncate the command buffer.

## Requirements

1. **R1**: Remove the duplicate `#define CUI_DRAW_BUF_MAX 1024` from `src/core/render.c`.
2. **R2**: Verify `render.c` compiles correctly using the canonical definition from `draw_cmd.h`.
3. **R3**: All existing tests MUST continue to pass.

## Scope

- **In scope**: Delete one line from `src/core/render.c`.
- **Out of scope**: Changing the value of `CUI_DRAW_BUF_MAX`, making the buffer dynamic.

## Acceptance Criteria

- `grep -n 'CUI_DRAW_BUF_MAX' src/core/render.c` returns zero matches (no local definition).
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
