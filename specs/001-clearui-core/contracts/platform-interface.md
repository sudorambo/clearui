# Contract: Platform Adapter (cui_platform)

The platform interface abstracts windowing, input, clipboard, cursor, and GPU surface creation. ClearUI does not own the event loop; the application polls events and calls `cui_begin_frame` / `cui_end_frame`. The application provides a `cui_platform` struct (or ClearUI uses a built-in adapter such as SDL3).

## Required operations (conceptual)

- **Window**: Create window (title, width, height); get size; destroy.
- **Events**: Poll events; map to ClearUI input (mouse, keyboard, focus, close). Return “running” flag for loop.
- **Clipboard**: Get/set clipboard text (optional; may be no-op on some platforms).
- **Cursor**: Set cursor shape (arrow, text, etc.) (optional).
- **GPU surface**: Create a surface (e.g. for Vulkan/Metal) bound to the window; destroy surface. Used by RDI for swapchain/surface.

## Contract guarantees

- **ClearUI expects**: All function pointers either implemented or NULL (NULL means “not supported” and ClearUI degrades gracefully where possible). Platform adapter runs on the same thread as `cui_ctx`.
- **Application owns**: The event loop; when to call `cui_begin_frame` and `cui_end_frame`; window lifetime.

## Built-in adapters

- **SDL3**: Full support; recommended default.
- **GLFW**: Desktop; lightweight.
- **Native (Win32/Cocoa/X11/Wayland)**: Zero extra deps; built incrementally per platform.
- **Emscripten**: Browser deployment.

## Versioning

Adding new required platform operations is MINOR or MAJOR; existing adapters must be updated or stubbed. Removing operations is MAJOR.
