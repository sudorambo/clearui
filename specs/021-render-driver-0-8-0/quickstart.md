# Quickstart: Render Driver (0.8.0)

## Using the software RDI

1. Create context and set the software RDI as you would any RDI:
   ```c
   cui_config config = { .draw_buf_capacity = 4096, ... };
   cui_ctx *ctx = cui_create(&config);
   const cui_rdi *rdi = cui_rdi_soft_get();
   cui_rdi_context *rdi_ctx = NULL;
   if (rdi->init(&rdi_ctx) != 0) { /* handle error */ }
   cui_set_rdi(ctx, rdi, rdi_ctx);
   ```
2. If you want the frame to appear on screen (rather than headless), use a platform adapter that supports the optional **present_software** callback. After creating the platform context and setting the RDI on the ClearUI context, call `cui_rdi_soft_set_viewport(rdi_ctx, width, height)` with the window size (e.g. from `platform->window_get_size`) and `cui_rdi_soft_set_platform(rdi_ctx, platform_ctx, platform)` so that `present()` can blit the framebuffer. See `contracts/platform-present-software-0.8.md`.
3. Each frame: `cui_begin_frame`, build your UI, `cui_end_frame` (which runs layout, build draw buffer, submit, present). The software RDI will rasterize the draw commands into its framebuffer and, if wired, present it to the window.

## Default font for text rendering

- The library uses a TTF font for measurement (see `src/font/atlas.c`) and, when the software RDI is used, for text rasterization.
- **Path**: The default path is typically `deps/default_font.ttf`. If this file is missing, text measurement falls back to approximate metrics and software text rendering may skip or use a fallback.
- **Providing your own**: Place a TTF at the expected path, or build with a path override if the build supports it. See README or `deps/README.md` for the exact path and any build options.

## Visual regression tests

- A test (or script) renders a fixed UI with the software RDI and compares the framebuffer to a baseline (e.g. checksum or reference image).
- Run with the rest of the test suite (e.g. `make unit-tests` or `make integration-tests`). If the baseline is updated (e.g. after intentional rendering changes), regenerate or update the stored checksum/reference in the repo.

## GPU RDI (future)

- This milestone focuses on the software RDI. A GPU backend (Vulkan, Metal, OpenGL, or WebGPU) can be added in a later release; it would implement the same `cui_rdi` interface and consume the same draw command buffer.
