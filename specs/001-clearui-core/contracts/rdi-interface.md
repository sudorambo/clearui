# Contract: Render Driver Interface (RDI)

The RDI is a struct of function pointers that ClearUI calls to perform GPU work. The application (or the build) provides one RDI implementation (Vulkan, Metal, WebGPU, or software). ClearUI never calls a graphics API directly; it only calls through the RDI.

## Required operations (conceptual)

- **Init / shutdown**: Initialize driver and resources; shutdown and release all resources.
- **Texture**: Create texture from RGBA data; upload subregion; destroy texture.
- **Draw commands**: Submit a command buffer produced by ClearUI. Command types include: clear, filled rect, rounded rect, line, textured quad, text (glyph quads), scissor.
- **Present**: Swap/present the current back buffer; optionally wait for vsync.

## Contract guarantees

- **ClearUI provides**: A single command buffer per frame (flat array of tagged unions); no dynamic allocation per command. Coordinates in logical pixels; RDI or platform may scale to physical pixels.
- **Driver implements**: All function pointers non-null when passed to context; thread-safe only if documented (typically single-threaded from UI thread). Driver MAY batch or reorder draws (e.g. by texture) for performance.
- **Software fallback**: `clearui_rdi_soft` must run without GPU; used for headless CI and testing.

## Out of scope

- Shader format or pipeline layout (driver-specific).
- Exact function signatures (defined in `clearui_rdi.h` or equivalent). This contract describes responsibilities only.

## Versioning

Adding new required RDI operations is a MINOR or MAJOR change; existing drivers must be updated or stubbed. Removing operations is MAJOR.
