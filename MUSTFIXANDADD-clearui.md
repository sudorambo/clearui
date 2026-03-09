# ClearUI 1.0.0 — Must Fix & Add

## Current State

ClearUI is a software-rendered immediate-mode GUI library. It rasterizes to an
RGBA framebuffer in CPU memory via its "soft RDI" (Render Device Interface).
There is no GPU rendering path — the ROADMAP mentions a future GPU RDI (Vulkan,
Metal) but it is unimplemented.

For Vulkan projects (e.g. the nexus32 emulator debug overlay), the consumer must:

1. Build ClearUI and call the soft RDI each frame to obtain a raw RGBA pixel buffer.
2. Upload that buffer to a Vulkan texture.
3. Write and compile their own vertex and fragment shaders.
4. Set up an entire Vulkan compositing pipeline (descriptor sets, render pass,
   fullscreen quad draw) from scratch.

ClearUI provides no shaders, no Vulkan example, and no pre-compiled SPIR-V.

---

## Critical Issues

### 1. No Shaders Shipped

ClearUI contains zero shader files — no `.vert`, `.frag`, `.glsl`, or `.spv`.
Any Vulkan (or OpenGL) consumer must write the entire overlay compositing shader
pair from scratch just to display the framebuffer ClearUI produces.

**Fix:** Ship a reference `overlay.vert` / `overlay.frag` pair in GLSL that
draws a fullscreen textured quad sampling the ClearUI RGBA texture. Include
RGBA-to-BGRA swizzle as a togglable option or variant.

### 2. No Pre-compiled SPIR-V

Even if GLSL source were provided, consumers would still need
`glslangValidator` or `shaderc` in their build toolchain to compile it.

**Fix:** Ship pre-compiled `.spv` files alongside the GLSL source so consumers
can integrate without any shader compiler dependency. Optionally embed the
SPIR-V as C byte arrays in a header (e.g. `clearui_overlay_spv.h`).

### 3. No Vulkan Integration Example

`docs/INTEGRATION_OVERLAY.md` describes the soft RDI workflow and mentions
Vulkan only in passing (pixel format note about BGRA). There is no example of
texture upload, pipeline creation, descriptor set binding, or render pass
integration.

**Fix:** Provide a self-contained Vulkan overlay example (~200–300 lines of C)
demonstrating:
- Vulkan texture creation and RGBA buffer upload
- Pipeline and descriptor set setup for the overlay quad
- Drawing the overlay after the main scene render pass

### 4. Hardcoded Font Path

`src/font/atlas.c` loads the font from `"deps/default_font.ttf"` resolved
relative to the process working directory. When ClearUI is embedded in another
project (e.g. the emulator runs from `nexus32-emulator/build/`), the font file
is not found.

**Fix:** Add one or more of:
- `cui_set_font_path(const char *path)` — runtime override
- `cui_load_font_memory(const void *data, size_t len)` — load from a buffer
- Build-time option to embed the font as a C byte array (similar to the SPIR-V
  embedding approach)

---

## High-Priority Improvements

### 5. `make lib` Excludes RDI and Platform Stub

`make lib` produces `libclearui.a` without `clearui_rdi_soft.c` or
`cui_platform_stub.c`. Overlay consumers who link only `libclearui.a` get
unresolved symbols. `docs/DROP_IN.md` explains the workaround but it is easy to
miss.

**Fix:** Either include the soft RDI and platform stub in the default archive,
or add a dedicated `make overlay` target that produces a ready-to-use static
library for overlay integration.

### 6. RGBA vs BGRA Guidance Without Code

`docs/INTEGRATION_OVERLAY.md` notes that Vulkan swapchains typically use BGRA
and suggests "swap R/B or use a shader swizzle" — but provides no code for
either approach.

**Fix:** The reference overlay fragment shader (issue 1) should handle this.
Provide both an RGBA and a BGRA variant, or a single shader with a push
constant / specialization constant toggle.

---

## Medium-Priority Improvements

### 7. No Install Target

There is no `cmake --install` target, no `make install`, and no `clearui.pc`
for pkg-config. System-wide or package-manager integration is unsupported.

**Fix:** Add a CMake install target for the library and public headers. Generate
a `clearui.pc` or CMake config file so downstream projects can use
`find_package(ClearUI)`.

### 8. No Embedded Font Option

The default font (`deps/default_font.ttf`) is a runtime dependency loaded from
disk. For single-binary distribution (common for emulators and tools), this is
inconvenient.

**Fix:** Provide a build option (e.g. `CLEARUI_EMBED_FONT`) that converts the
TTF to a C byte array at build time and links it into the library. A simple
`xxd -i` or CMake custom command would suffice.

---

## Summary Checklist

| # | Item | Priority | Status |
|---|------|----------|--------|
| 1 | Ship reference overlay shaders (GLSL) | Critical | Missing |
| 2 | Ship pre-compiled SPIR-V (+ optional C header embed) | Critical | Missing |
| 3 | Vulkan integration example | Critical | Missing |
| 4 | Configurable / embeddable font path | Critical | Missing |
| 5 | Fix `make lib` to include soft RDI for overlay use | High | Missing |
| 6 | RGBA/BGRA shader variant | High | Missing |
| 7 | CMake install target and pkg-config | Medium | Missing |
| 8 | Embedded font build option | Medium | Missing |
