# Contract: Theme API (0.6.0)

## Public API

### cui_theme struct

```c
typedef struct cui_theme {
    unsigned int text_color;
    unsigned int button_bg;
    unsigned int checkbox_bg;
    unsigned int input_bg;
    float        corner_radius;
    int          font_size;
    unsigned int focus_ring_color;
    float        focus_ring_width;
} cui_theme;
```

- **Semantics**: All colors are 0xAARRGGBB. Values are used by the library for default widget colors, corner radius, font size for layout/measurement, and focus ring drawing. When a widget has an explicit style override (e.g. `cui_label_styled`), that override takes precedence for that widget; theme supplies defaults when no override is set.
- **Lifetime**: App may use stack or static storage; when passed to `cui_set_theme`, the library copies the struct. App does not need to keep it alive after the call.

### cui_set_theme

```c
void cui_set_theme(cui_ctx *ctx, const cui_theme *theme);
```

- **Semantics**: Sets the context’s current theme. If `theme` is non-NULL, the library copies `*theme` into the context; subsequent frames use this theme for draw and layout. If `theme` is NULL, the context is reset to the built-in default theme (same as the previous compile-time default).
- **Thread safety**: Same as rest of API; single-threaded only.
- **Null ctx**: If `ctx` is NULL, no-op.

### Dark preset

```c
void cui_theme_dark(cui_theme *out);
```

- **Semantics**: Fills `*out` with a dark theme (dark backgrounds, light text). Does not allocate. App then calls `cui_set_theme(ctx, out)` to apply. Focus ring remains visible (WCAG 2.1 AA).

## Internal use

- **cui_ctx_theme(ctx)**: Internal accessor (e.g. in `context.h`) returning `const cui_theme *` for use by `draw_cmd.c` and `layout.c`. Never returns NULL; returns pointer to context’s theme slot (always initialized to default).

## Default theme

- The built-in default theme has the same numeric values as the current `theme.h` #defines: light backgrounds, black text, blue focus ring, 4px corner radius, 16px font size, 2px focus ring width. This ensures no visual or behavioral change when no theme is set.
