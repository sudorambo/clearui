# Quickstart: Scaled buffer on context

## Summary

This refactor is internal: the scaled draw command buffer used for Hi-DPI is no longer a file-static in `render.c` but a member of the context, with the same lifecycle as the main draw buffer. There is no change to the public API or to how applications use ClearUI.

## For implementers

1. **Context struct** (`src/core/context.c`): Add `cui_draw_command_buffer scaled_buf` to `struct cui_ctx`.
2. **Create**: After init of `draw_buf` and `canvas_cmd_buf`, call `cui_draw_buf_init(&ctx->scaled_buf, cap)`. On failure, fini all and return NULL.
3. **Destroy**: Call `cui_draw_buf_fini(&ctx->scaled_buf)` together with the other buffers.
4. **Getter**: Add `cui_draw_command_buffer *cui_ctx_scaled_buf(cui_ctx *ctx)` in context.c and declare it in context.h.
5. **Render** (`src/core/render.c`): Remove any `static cui_draw_command_buffer scaled_buf`. When scale != 1.f, use `cui_ctx_scaled_buf(ctx)` as the destination of `scale_buf` and pass it to RDI submit.

## Verification

- `grep -n 'static.*scaled_buf\|static.*cui_draw_command_buffer' src/core/render.c` should return no matches.
- `make clean && make all && make unit-tests && make integration-tests` passes.
