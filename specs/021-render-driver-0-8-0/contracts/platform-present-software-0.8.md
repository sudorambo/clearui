# Contract: Platform present_software (0.8.0)

Optional extension to the platform adapter interface for software RDI present.

## Purpose

When the application uses the software RDI (no GPU), the RDI produces an RGBA framebuffer each frame. To display it, the platform adapter must copy that buffer to the window. This contract defines an optional callback on `cui_platform` for that purpose.

## Signature (conceptual)

```c
void (*present_software)(cui_platform_ctx *ctx, const void *rgba, int width, int height, int pitch_bytes);
```

- **ctx**: Platform context (e.g. SDL window/session).
- **rgba**: Pointer to the framebuffer (RGBA, 8 bpp per channel). Valid only for the duration of the call.
- **width**, **height**: Framebuffer dimensions in pixels.
- **pitch_bytes**: Byte stride per row (typically width * 4). Use for non-contiguous or padded buffers.

## Behavior

- If the platform supports software present (e.g. SDL3 with a texture or surface that can be updated from CPU), it implements this callback: copy `rgba` to the display (e.g. SDL_UpdateTexture + SDL_RenderCopy, or lock surface and memcpy), then present the frame.
- If not supported (e.g. headless, or GPU-only platform), set the pointer to NULL. The software RDI’s present() will then be a no-op.
- ClearUI core does not call this; the software RDI implementation calls it from its present() when it has a reference to the platform.

## Wiring

- The application sets both the RDI and the platform on the context. The software RDI init (or a separate setter) may accept (platform_ctx, platform) so that present() can invoke platform->present_software(platform_ctx, fb, w, h, pitch). Exact wiring is implementation-defined.
