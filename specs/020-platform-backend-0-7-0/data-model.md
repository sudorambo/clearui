# Data Model: Platform Backend (0.7.0)

Entities and relationships for the platform adapter. No change to ClearUI core data structures; this describes the adapter’s view and the existing platform contract.

## Entities

### cui_platform (existing)

- **Role**: Function-pointer struct defined in `clearui_platform.h`; abstracts window, events, clipboard, cursor, GPU surface.
- **Fields (conceptual)**:
  - `window_create(out_ctx, title, width, height)` → int
  - `window_destroy(ctx)`
  - `window_get_size(ctx, width, height)`
  - `poll_events(ctx)` → bool (keep running?)
  - Optional: `clipboard_get`, `clipboard_set`, `cursor_set`, `surface_get`, `surface_destroy`
- **Extension for 0.7.0**: Optional `scale_factor_get(ctx)` → float (NULL = not supported) so adapters can report Hi-DPI scale; otherwise app sets `cui_config.scale_factor` from platform APIs.

### cui_platform_ctx (existing)

- **Role**: Opaque context created by the adapter in `window_create`; passed to every platform callback and to `cui_set_platform(ctx, platform, platform_ctx)`.
- **Ownership**: Adapter allocates in window_create, adapter (or app) frees in window_destroy. ClearUI never frees it.
- **Contents**: Adapter-specific (e.g. SDL_Window* and related state for SDL3).

### SDL3 adapter state (new, in cui_platform_sdl3.c)

- **Role**: Private state for the SDL3 implementation.
- **Typical fields**: SDL_Window*, SDL_GLContext or other GPU context if needed by RDI, last window size, maybe clipboard/cursor cache. All behind the opaque `cui_platform_ctx` (e.g. a struct cast to `cui_platform_ctx*`).

### Event mapping (conceptual)

- **Role**: Adapter maps platform events to ClearUI input:
  - Mouse move → `cui_inject_mouse_move(ctx, x, y)`
  - Mouse button → `cui_inject_click(ctx, x, y)`
  - Scroll → `cui_inject_scroll(ctx, dx, dy)`
  - Key / char → `cui_inject_key` / `cui_inject_char`
  - Resize → next `window_get_size` returns new size; app or adapter may update `cui_config` or a viewport.
  - Close → `poll_events` returns false so app exits loop.
- No new entities; mapping is code in the adapter’s `poll_events` (or equivalent) that calls existing ClearUI inject APIs.

## State transitions

- **Window lifecycle**: None → window_create → [poll_events loop] → window_destroy → None.
- **Context lifecycle**: App creates `cui_ctx` (with or without platform); when using adapter, app calls `cui_set_platform(ctx, &platform, platform_ctx)` before the first frame. Platform context must outlive all frames until `cui_set_platform(ctx, NULL, NULL)` or context destroy.

## Validation rules

- All required `cui_platform` pointers (window_create, window_destroy, window_get_size, poll_events) must be non-NULL for a real adapter.
- Optional pointers may be NULL; ClearUI must not dereference them when NULL.
- `scale_factor_get` if present must return a positive value (e.g. 1.0 minimum); adapter may clamp.
