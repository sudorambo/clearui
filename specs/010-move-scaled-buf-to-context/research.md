# Research: Move static scaled_buf to context struct

## Decision: Store scaled buffer on context

**Decision**: Add a `scaled_buf` member to `struct cui_ctx`, initialize it in `cui_create` with the same capacity as the main draw buffer, finalize it in `cui_destroy`, and expose it via an internal getter `cui_ctx_scaled_buf(ctx)`. In `cui_render_submit`, when scale != 1.f, use this buffer instead of a file-static in render.c.

**Rationale**: The Constitution requires no implicit global state (Clarity) and explicit ownership (Ownership). A file-static buffer in render.c is global state and its lifetime is unrelated to the context. Putting the buffer on the context makes ownership clear and removes static state from the render path.

**Alternatives considered**:
- **Keep static in render.c**: Rejected; violates no-implicit-global-state and blurs ownership.
- **Allocate scaled buffer in submit when scale != 1.f**: Rejected; would add allocation in the hot path; pre-allocating on context avoids that and keeps lifecycle with the context.
- **Single shared buffer for draw and scaled**: Rejected; would require double-buffering or overwriting the main buffer before submit; separate scaled_buf is simpler and matches current behavior.
