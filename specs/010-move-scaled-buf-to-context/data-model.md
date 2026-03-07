# Data Model: Scaled buffer on context

## Entity: cui_ctx (extended)

| Member (relevant) | Type | Description |
|------------------|------|-------------|
| draw_buf | cui_draw_command_buffer | Main draw command buffer (existing). |
| canvas_cmd_buf | cui_draw_command_buffer | Canvas-specific draw buffer (existing). |
| **scaled_buf** | **cui_draw_command_buffer** | **New.** Buffer used by render when scale != 1.f; same capacity as draw_buf. |

## Lifecycle

- **Create**: After initializing `draw_buf` and `canvas_cmd_buf` with capacity `cap`, initialize `scaled_buf` with the same `cap`. If any init fails, fini all and return NULL.
- **Destroy**: Before freeing the context, call `cui_draw_buf_fini` on `draw_buf`, `canvas_cmd_buf`, and `scaled_buf`.
- **Submit (scale != 1.f)**: `scale_buf(main_buf, ctx->scaled_buf, scale)` then submit `ctx->scaled_buf`. No allocation in submit.

## Getter

- `cui_ctx_scaled_buf(cui_ctx *ctx)`: Returns `ctx ? &ctx->scaled_buf : NULL`. Internal use (e.g. render.c); not required in public API.
