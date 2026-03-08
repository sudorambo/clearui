# Feature Specification: Render Driver (0.8.0)

**Version**: 0.8.0  
**Branch**: `021-render-driver-0-8-0`

## Goal

Ship at least one real RDI backend. The software RDI is currently a no-op; this milestone implements it to rasterize to an RGBA framebuffer and present to the platform, or documents an alternative (e.g. GPU RDI). Visual regression test infrastructure and default font handling are in scope.

## User Stories

1. **US1** As an application developer, I want the software RDI to draw filled rects, rounded rects, lines, and text so that my UI is visible when I use the default build without a GPU backend.
2. **US2** As a developer, I want scissor clipping so that scroll regions and overflow are correctly clipped.
3. **US3** As a developer, I want the software RDI to present the framebuffer to the window (e.g. blit to platform surface) so that the rendered frame is visible.
4. **US4** As a maintainer, I want visual regression tests (e.g. render to PNG, compare) so that rendering changes are detected.
5. **US5** As a user of the library, I want a default TTF font or clear documentation on how to provide one so that text renders out of the box or is easy to configure.

## Acceptance Criteria

- Software RDI implements `submit()`: rasterize all command types (rect, rounded_rect, line, text, honor scissor) into an internal or provided RGBA framebuffer.
- Text rendering uses stb_truetype glyph bitmaps (or existing font atlas path) for CUI_CMD_TEXT.
- Scissor commands clip subsequent draws until the next scissor or end of buffer.
- Present: framebuffer is blitted to the platform surface when available (e.g. via platform callback or equivalent); otherwise no-op for headless.
- Visual regression: at least one test or script can render a known UI to a buffer/PNG and compare to a baseline (hash or image diff).
- Default font: either ship a TTF in the repo (e.g. deps/) or document path/config and fallback behavior in README/spec.

## Out of Scope (0.8.0)

- Full GPU RDI (Vulkan/Metal/OpenGL/WebGPU) is an alternative mentioned in the roadmap; this milestone prioritizes the software RDI. GPU can be a follow-up.
- Complex text shaping (e.g. HarfBuzz) remains optional/future.

## Requirements

- **R1** Software RDI must not require C++; C11 only, vendored stb_truetype acceptable.
- **R2** Default build (no GPU) must be able to show a window with visible UI when a platform adapter (e.g. SDL3) and software RDI are used.
- **R3** Visual regression infrastructure must run in CI where possible (e.g. headless render to buffer, compare checksum or PNG).
