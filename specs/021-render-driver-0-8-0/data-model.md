# Data Model: Render Driver (0.8.0)

## Entities

### Software RDI context (soft_ctx)

- **Role**: Per-context state for the software RDI implementation.
- **Fields** (conceptual):
  - `framebuffer`: RGBA pixel buffer (unsigned char * or equivalent), width × height × 4.
  - `width`, `height`: Framebuffer dimensions; updated on first submit or resize (e.g. from platform or submit viewport).
  - `scissor_x`, `scissor_y`, `scissor_w`, `scissor_h`: Current scissor rectangle (pixel coords); initialized to full buffer.
  - Optional: reference to platform (platform_ctx, platform) or present callback for present().
  - Optional: font rasterization cache (e.g. stbtt font info or baked atlas) for TEXT commands.
- **Lifecycle**: Allocated in RDI init, freed in shutdown. Framebuffer (re)allocated when dimensions change.

### Draw command buffer (existing)

- **Role**: Input to RDI submit(); produced by Core. Tagged union commands: RECT, ROUNDED_RECT, LINE, TEXT, SCISSOR.
- **Invariant**: Coordinates in logical pixels; software RDI uses them as-is or scales if it owns a fixed-size buffer (scale from context or first submit).

### Platform present_software (new optional callback)

- **Signature** (conceptual): `void (*present_software)(cui_platform_ctx *ctx, const void *rgba, int width, int height, int pitch_bytes);`
- **Role**: Platform adapter copies the RGBA framebuffer to the window (e.g. SDL_UpdateTexture + render, or lock surface + memcpy). NULL if not supported (headless).

### Visual regression baseline

- **Role**: Stored reference (checksum or PNG path) for the test that renders a fixed UI and compares output.
- **Storage**: In repo (e.g. `tests/baselines/` or a constant expected checksum in the test). Updated when rendering is intentionally changed.

## State transitions

1. **RDI init**: Create soft_ctx; framebuffer may be NULL until first submit (or init accepts dimensions).
2. **Submit**: If framebuffer is NULL or size changed, (re)allocate framebuffer. Clear or retain previous frame (clear for 0.8.0). For each command: apply scissor; rasterize rect/rounded_rect/line/text into framebuffer.
3. **Scissor command**: Update soft_ctx scissor to command rect.
4. **Present**: If platform provides present_software, call it with framebuffer pointer and dimensions; else no-op.
5. **Shutdown**: Free framebuffer and soft_ctx.

## Validation rules

- Framebuffer dimensions must be positive; submit must not write outside bounds.
- Scissor must be clipped to framebuffer; draw primitives clipped to current scissor.
- TEXT commands: string pointers valid until after submit returns (caller guarantees; frame allocator in Core).
