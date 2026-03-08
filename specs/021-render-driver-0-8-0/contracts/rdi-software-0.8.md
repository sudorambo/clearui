# Contract: Software RDI (0.8.0)

This contract specifies the behavior of the software RDI implementation (`cui_rdi_soft_get()`). It extends the generic [RDI contract](../../001-clearui-core/contracts/rdi-interface.md).

## Responsibilities

- **init**: Allocate driver context. Caller must call `cui_rdi_soft_set_viewport(rdi_ctx, width, height)` before submit so the framebuffer is allocated; otherwise submit is a no-op. Present is a no-op until platform is wired (US3).
- **shutdown**: Free context and any owned framebuffer and font cache.
- **texture_create / texture_upload / texture_destroy**: May remain no-op or stub for 0.8.0 (software path does not use texture API for UI; glyphs are rasterized into the framebuffer).
- **submit**: Rasterize the command buffer into an internal RGBA framebuffer. Command order is preserved. For each command:
  - **CUI_CMD_RECT**: Fill rectangle with color (pixel bounds from x, y, w, h).
  - **CUI_CMD_ROUNDED_RECT**: Fill rounded rectangle (radius r, color).
  - **CUI_CMD_LINE**: Draw line from (a.x, a.y) to (b.x, b.y), thickness, color.
  - **CUI_CMD_TEXT**: Rasterize text at (x, y) with color using stb_truetype (or atlas); clip to current scissor.
  - **CUI_CMD_SCISSOR**: Set current scissor rectangle for subsequent draws.
- **scissor**: If called by ClearUI, set driver scissor (may be redundant with CUI_CMD_SCISSOR; both honored).
- **present**: If a present target (platform callback or equivalent) was provided, hand the framebuffer (rgba, width, height, pitch) to the platform for blit. Otherwise no-op.

## Framebuffer

- Dimensions: Either fixed at init or derived from first submit (e.g. from platform window size or config). Must be positive.
- Format: RGBA, 8 bits per channel, row-major, pitch = width * 4 unless otherwise documented.
- Clearing: Framebuffer is cleared at the start of each submit (or undefined if not cleared; 0.8.0 specifies clear for predictable regression tests).

## Platform integration

- To show the software-rendered frame on screen, the application must either:
  - Use a platform that implements the optional **present_software** callback (see contract `platform-present-software-0.8.md`), and the software RDI must be given a reference to that platform at init or via a setter, or
  - Manually read the framebuffer from the RDI (if an accessor is provided) and blit via platform APIs.

## Guarantees

- Single-threaded: submit/present called from UI thread only.
- Command buffer pointers (e.g. text) are valid for the duration of submit() only.
- No new public API beyond existing RDI; init may be extended with optional parameters without breaking existing callers.
