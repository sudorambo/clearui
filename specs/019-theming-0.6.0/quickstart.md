# Quickstart: Theming (0.6.0)

**Branch**: `019-theming-0.6.0`

## Using runtime theme

Set a theme before building frames; the library uses it for default colors, corner radius, font size, and focus ring:

```c
#include "clearui.h"

cui_ctx *ctx = cui_create(&(cui_config){ .title = "App", .width = 400, .height = 300 });

cui_theme theme = {
    .text_color         = 0xff000000u,
    .button_bg          = 0xffe0e0e0u,
    .checkbox_bg        = 0xffffffffu,
    .input_bg           = 0xffffffffu,
    .corner_radius      = 4.f,
    .font_size          = 16,
    .focus_ring_color   = 0xff0066ccu,
    .focus_ring_width   = 2.f,
};
cui_set_theme(ctx, &theme);

// ... run frame loop; all widgets use this theme ...
```

## Dark mode (preset)

```c
cui_theme dark;
cui_theme_dark(&dark);
cui_set_theme(ctx, &dark);
```

## Reset to default

```c
cui_set_theme(ctx, NULL);  // use built-in light default again
```

## Running the theme test

After implementation:

```bash
make test_theme && ./test_theme
```

The test sets a custom theme (e.g. distinct button_bg), builds a frame with a button, and asserts the draw buffer contains the theme’s button background color.
