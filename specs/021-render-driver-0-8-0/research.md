# Research: Render Driver (0.8.0)

## R1: Software RDI rasterization approach

**Decision**: Implement rasterization inside `clearui_rdi_soft.c`: own an RGBA framebuffer (width × height × 4), iterate the draw command buffer in order. For each command type:
- **RECT**: Fill axis-aligned rectangle with color (blend or overwrite; 0.8.0 can use overwrite for simplicity).
- **ROUNDED_RECT**: Fill rounded rectangle (e.g. approximate with rectangles + circles at corners, or a simple in-rect test with rounded corners).
- **LINE**: Bresenham or similar; thickness via repeated 1px lines or filled thin quad.
- **TEXT**: Use stb_truetype to rasterize glyphs (e.g. `stbtt_GetCodepointBitmap` or pre-baked atlas from `stbtt_PackFontRanges`); blend glyph bitmap onto framebuffer at (x, y) with color; advance x by advance width. Reuse font data from existing atlas (same TTF path) or accept font_id 0 only and use atlas’s stbtt_fontinfo.
- **SCISSOR**: Set current scissor rectangle; all subsequent draws are clipped to the intersection of current scissor and draw bounds.

**Rationale**: Single file, no GPU; matches existing draw command format. stb_truetype is already used for metrics in `src/font/atlas.c`; adding rasterization there or in RDI keeps dependencies minimal.

**Alternatives considered**: Offload to a separate “renderer” library (more deps, rejected). GPU-only path (deferred to later milestone).

---

## R2: Present / blit to platform surface

**Decision**: Extend `cui_platform` with an optional **present_software**(platform_ctx, rgba_ptr, width, height, pitch_bytes). When the software RDI’s `present()` is called, if the RDI context holds a reference to the platform (e.g. set at init or via a separate set_platform call), call this callback to hand the framebuffer to the platform for blit/display. If the platform does not provide present_software (NULL), `present()` is a no-op (headless/testing). The software RDI init may be extended to accept optional (platform_ctx, platform) or a present callback so it knows where to send the buffer.

**Rationale**: Keeps RDI independent of platform type; platform adapter (e.g. SDL3) implements blit (e.g. SDL_UpdateTexture + SDL_RenderCopy, or lock surface and memcpy). No change to core ClearUI context; app wires RDI and platform as today.

**Alternatives considered**: RDI init takes (ctx, platform_ctx, platform) so soft RDI can call platform->present_software. Or: soft RDI exposes get_framebuffer() and the app is responsible for blitting (pushes complexity to every app; rejected).

---

## R3: Scissor clipping

**Decision**: Maintain a current scissor rect (initially full framebuffer). On CUI_CMD_SCISSOR, set current scissor to the command’s rect (in pixel coordinates). For each draw command (rect, line, text, rounded_rect), clip the primitive to the current scissor (e.g. intersect rects; for lines, clip to scissor box; for text, clip each glyph blit rect). No need for hierarchical scissor stack if the command buffer is already emitted in order; one active scissor is sufficient for 0.8.0.

**Rationale**: Matches common immediate-mode behavior; single scissor keeps implementation simple.

**Alternatives considered**: Scissor stack (more flexible, deferred if needed).

---

## R4: Visual regression test infrastructure

**Decision**: Add a test that: (1) creates a context with software RDI, (2) builds a fixed small UI (e.g. one frame with a known set of widgets), (3) runs layout and draw, (4) calls submit so the soft RDI rasterizes into its framebuffer, (5) reads back the framebuffer (or a region) and compares to a baseline (checksum of pixel data, or a small number of sampled pixels). Optionally: write framebuffer to PNG (e.g. stb_image_write if vendored) and diff against reference PNG in repo; if no PNG writer, checksum is sufficient for 0.8.0. Test is run in CI; baseline updated when rendering is intentionally changed.

**Rationale**: Catches regressions in soft RDI and draw command emission. Checksum is dependency-free; PNG reference allows visual inspection and can be added later.

**Alternatives considered**: Full image diff in CI (heavier; can add later). No regression test (rejected; spec requires it).

---

## R5: Default TTF font

**Decision**: Document in README and/or `deps/README.md`: (1) the library expects a TTF at a documented path (e.g. `deps/default_font.ttf`) for text measurement and, when using software RDI, for text rasterization; (2) if the file is missing, measurement falls back to approximate metrics (existing behavior) and software RDI text can skip rendering or use a fallback (e.g. rectangle). Optionally ship a minimal, freely licensed TTF (e.g. one of the SIL Open Font License or similar) in `deps/` so that “clone, build, run” shows text without user action. If we do not bundle a font, the spec’s “document how to provide one” is satisfied by README + path constant (e.g. in atlas or build-time config).

**Rationale**: Avoids licensing or repo size issues from bundling; satisfies “ship default or document”. If we bundle one small TTF, zero-to-window improves.

**Alternatives considered**: Mandatory bundled font (simplest UX; chosen if we find a small OFL font). No default path (worse DX; rejected).
