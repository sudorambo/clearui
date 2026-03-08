# Quickstart: Platform Backend (0.7.0)

## Using the SDL3 adapter

1. **Build with SDL3** (when available):
   - Ensure SDL3 is installed (e.g. `sdl3-config` or pkg-config).
   - Build: `make all WITH_SDL3=1` (or equivalent per Makefile).
   - Link your app with the SDL3 adapter object and SDL3.

2. **Minimal loop**:
   - Create window with the adapter: `platform->window_create(&platform_ctx, "App", 800, 600)`.
   - Create ClearUI context: `ctx = cui_create(&config)`.
   - Set platform: `cui_set_platform(ctx, platform, platform_ctx)`.
   - Optional: set `config.scale_factor` from `platform->scale_factor_get(platform_ctx)` if non-NULL before or after creating ctx (or update when resize happens).
   - Loop: while `platform->poll_events(platform_ctx)` do `cui_begin_frame(ctx)`, build UI, `cui_end_frame(ctx)`, then render (RDI) and swap.
   - On exit: `cui_destroy(ctx)`, `platform->window_destroy(platform_ctx)`.

3. **Integration test**:
   - Run the “real window, one frame, close” test when a display is available.
   - In headless CI it may skip with “no display.”

## Bringing your own platform

1. **Implement** the `cui_platform` struct (see `include/clearui_platform.h` and `specs/020-platform-backend-0-7-0/contracts/platform-adapter-0.7.md`).
2. **Required**: `window_create`, `window_destroy`, `window_get_size`, `poll_events`.
3. **Optional** (set to NULL if not supported): `clipboard_get`, `clipboard_set`, `cursor_set`, `surface_get`, `surface_destroy`, and if added, `scale_factor_get`.
4. **In poll_events**: Map OS events to ClearUI: mouse → `cui_inject_mouse_move` / `cui_inject_click`, scroll → `cui_inject_scroll`, key/char → `cui_inject_key` / `cui_inject_char`. Return false when the app should quit.
5. **Register**: `cui_set_platform(ctx, &my_platform, my_platform_ctx)` before the first frame.

## Stub (headless / CI)

- Use `cui_platform_stub_get()` for tests and headless runs; no window is created, `poll_events` always returns true.
