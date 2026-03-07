# Research: Remove Duplicate CUI_DRAW_BUF_MAX

**Branch**: `004-remove-duplicate-draw-buf-max` | **Date**: 2026-03-07

## Investigation

### Where is CUI_DRAW_BUF_MAX defined?

| File | Line | Value | Used by |
|------|------|-------|---------|
| `src/core/draw_cmd.h` | 50 | 1024 | `cui_draw_command_buffer` struct, `draw_cmd.c` push functions |
| `src/core/render.c` | 10 | 1024 | `scale_buf` loop guard |

### Does render.c include draw_cmd.h?

Yes. `render.c` line 6: `#include "core/draw_cmd.h"`. The canonical definition is already visible.

### Decision

- **Decision**: Delete line 10 (`#define CUI_DRAW_BUF_MAX 1024`) from `render.c`.
- **Rationale**: The include already provides the constant. The duplicate is a maintenance hazard — if only one definition is updated, `scale_buf` could overflow or truncate silently.
- **Alternatives considered**: None needed. This is the only correct fix.
