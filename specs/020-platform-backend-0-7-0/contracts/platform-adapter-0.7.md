# Contract: Platform Adapter (0.7.0)

This contract extends the existing platform interface (see `specs/001-clearui-core/contracts/platform-interface.md`) for the 0.7.0 milestone: one built-in real adapter (SDL3) and documentation for custom adapters.

## Required operations (unchanged)

- **window_create**(out_ctx, title, width, height) → int. Allocate platform context, create window; on success set *out_ctx and return 0; on failure return non-zero and do not set *out_ctx.
- **window_destroy**(ctx). Free platform context and destroy window; no-op if ctx is NULL.
- **window_get_size**(ctx, width, height). Write current window size in logical pixels; safe if width or height is NULL.
- **poll_events**(ctx) → bool. Process events; map to ClearUI inject APIs as needed; return false to signal “quit” (e.g. window closed).

## Optional operations (NULL = not supported)

- **clipboard_get**(ctx, buf, size). Copy clipboard text into buf, null-terminated; return 0 on success.
- **clipboard_set**(ctx, text). Set system clipboard to text.
- **cursor_set**(ctx, shape). Set system cursor shape (e.g. arrow, text caret). Shape values TBD or from clearui_platform.h.
- **surface_get**(ctx), **surface_destroy**(ctx, surface). GPU surface for RDI; NULL if not used.

## Optional: scale factor (0.7.0)

- **scale_factor_get**(ctx) → float. Return display/window scale factor (e.g. 2.0 on Hi-DPI). If not present (NULL), app sets `cui_config.scale_factor` before creating the context or after window creation using platform APIs. Return value must be >= 1.0 (adapter may clamp).

## SDL3 adapter guarantees

- Implements all required and, where SDL3 supports it, optional operations (clipboard, cursor, scale).
- Uses SDL3 C API only; single-threaded; app owns the loop (adapter does not run a nested loop).
- On window creation failure, returns non-zero and does not set *out_ctx.

## Custom adapter (bring your own)

- Implement the required function pointers; set optional ones to NULL if not supported.
- Allocate `cui_platform_ctx` in window_create; free it in window_destroy.
- Call `cui_set_platform(ctx, &my_platform, my_ctx)` before the first `cui_begin_frame`.
- In poll_events, call `cui_inject_mouse_move`, `cui_inject_click`, `cui_inject_scroll`, `cui_inject_key`, `cui_inject_char` as events occur; return false when the app should exit.

## Versioning

Adding new required platform operations is MINOR or MAJOR per existing contract. Adding optional operations (e.g. scale_factor_get) is MINOR; adapters may set them to NULL.
