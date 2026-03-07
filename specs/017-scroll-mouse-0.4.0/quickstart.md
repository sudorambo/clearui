# Quickstart: Scroll & Mouse (0.4.0)

## Run tests

After implementation:

```bash
make test_scroll          # unit test: scroll clamping
make test_scroll_region   # integration: scroll + verify offset
make unit-tests           # includes test_scroll
make integration-tests    # includes test_scroll_region
```

## Minimal usage

### Scroll

```c
// Each frame: set mouse position (e.g. from platform), then inject scroll when wheel events occur
cui_inject_mouse_move(ctx, platform_mouse_x, platform_mouse_y);
if (platform_scroll_dy != 0)
    cui_inject_scroll(ctx, 0, platform_scroll_dy);

cui_begin_frame(ctx);
cui_center(ctx);
cui_scroll(ctx, "panel", &(cui_scroll_opts){ .max_height = 200 });  // id "panel" so it can receive scroll
    // ... children (e.g. labels, buttons) ...
cui_end(ctx);
cui_end_frame(ctx);
```

### Hover

```c
cui_inject_mouse_move(ctx, x, y);
// ... build UI ...
cui_end_frame(ctx);  // hover hit-test runs here

// Next frame (or same frame after end_frame): query hover
cui_begin_frame(ctx);
cui_center(ctx);
if (cui_ctx_is_hovered(ctx, "my_btn"))
    cui_push_style(ctx, &highlight_style);
cui_button(ctx, "my_btn");
cui_pop_style(ctx);
// ...
```

### Optional cursor

When hovered widget changes, call the platform’s cursor_set if available:

```c
const char *hovered = cui_ctx_hovered_id(ctx);
if (hovered && ctx->platform && ctx->platform->cursor_set)
    ctx->platform->cursor_set(ctx->platform_ctx, CURSOR_POINTER);  // example shape constant
```
