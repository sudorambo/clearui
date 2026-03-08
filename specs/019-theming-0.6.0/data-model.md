# Data Model: Theming (0.6.0)

**Branch**: `019-theming-0.6.0` | **Date**: 2026-03-07

## Entities

### cui_theme (public struct)

- **Location**: Declared in `include/clearui.h` (or in a dedicated theme header included by clearui.h).
- **Fields**: All values used by draw and layout from current `theme.h`:
  - `text_color` (unsigned int, 0xAARRGGBB)
  - `button_bg` (unsigned int)
  - `checkbox_bg` (unsigned int)
  - `input_bg` (unsigned int)
  - `corner_radius` (float)
  - `font_size` (int)
  - `focus_ring_color` (unsigned int)
  - `focus_ring_width` (float)
- **Ownership**: App creates and owns `cui_theme` values; when passed to `cui_set_theme`, the context takes a copy. No heap allocation by the library for theme.

### cui_ctx theme slot (internal)

- **Location**: `struct cui_ctx` in `src/core/context.c`.
- **Field**: `cui_theme theme` (or similar) — the current theme used for this context. Initialized in `cui_create` to the built-in default.
- **Invariant**: Always valid; after `cui_set_theme(ctx, NULL)` or before any set, it holds the built-in default (same semantics as current #define defaults).

### Built-in default theme

- **Representation**: Static const struct or a function that returns the default theme (same numeric values as current `CUI_THEME_DEFAULT_*` and `CUI_THEME_FOCUS_RING_*`). Used to initialize `ctx->theme` in `cui_create` and when `cui_set_theme(ctx, NULL)` is called.
- **Location**: `src/core/theme.h` and/or `src/core/context.c` (or a small `theme.c` if preferred).

### Dark preset

- **Representation**: A function `void cui_theme_dark(cui_theme *out)` (or `cui_theme cui_theme_dark(void)`) that fills or returns a theme with dark backgrounds and light text. Does not allocate; fills app- or stack-allocated struct.
- **Usage**: App calls it to obtain a theme struct, then calls `cui_set_theme(ctx, &theme)`.

## State transitions

- **Create context**: `ctx->theme` is set to built-in default.
- **cui_set_theme(ctx, &t)**: Copy `t` into `ctx->theme`.
- **cui_set_theme(ctx, NULL)**: Copy built-in default into `ctx->theme`.
- **Draw/Layout**: Read from `ctx->theme` (or from `cui_ctx_theme(ctx)`) only; no reference to compile-time macros at runtime.
