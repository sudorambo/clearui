# ClearUI API Reference

Public API as of 0.10.0. See `include/clearui.h`, `include/clearui_platform.h`, and `include/clearui_rdi.h` for full declarations.

## Version

| API | Description |
|-----|-------------|
| `cui_version_string(void)` | Returns runtime version string, e.g. `"0.10.0"`. Not thread-safe. |

## Context lifecycle

| API | Description |
|-----|-------------|
| `cui_create(const cui_config *config)` | Create context; NULL config uses defaults. |
| `cui_destroy(cui_ctx *ctx)` | Destroy context and free resources. |
| `cui_set_platform(ctx, platform, platform_ctx)` | Set platform adapter (window, events). |
| `cui_set_rdi(ctx, rdi, rdi_ctx)` | Set render driver. |
| `cui_running(ctx)` | Non-zero while app should keep running. |
| `cui_begin_frame(ctx)` | Start frame; call before emitting UI. |
| `cui_end_frame(ctx)` | End frame; run layout, diff, render. |
| `cui_run(ctx, ui_func)` | Run loop: poll_events, begin_frame, ui_func, end_frame, present until quit. |

## State and allocation

| API | Description |
|-----|-------------|
| `cui_state(ctx, key, size)` | Persistent block keyed by string; zero-initialized; valid for context lifetime. |
| `cui_frame_alloc(ctx, size)` | Per-frame bump allocation; valid until next cui_begin_frame. |
| `cui_frame_printf(ctx, fmt, ...)` | sprintf into frame buffer; max 64 KiB; valid until next cui_begin_frame. |

## Theming and style

| API | Description |
|-----|-------------|
| `cui_set_theme(ctx, theme)` | Set runtime theme; NULL = built-in default. |
| `cui_theme_dark(cui_theme *out)` | Fill *out with dark preset. |
| `cui_push_style(ctx, style)` | Push style for next widgets. |
| `cui_pop_style(ctx)` | Pop style. |

## Widgets

| API | Description |
|-----|-------------|
| `cui_label(ctx, text)` | Label. |
| `cui_label_styled(ctx, text, style)` | Label with style. |
| `cui_button(ctx, id)` | Button; returns 1 if clicked. |
| `cui_checkbox(ctx, id, &checked)` | Checkbox; returns 1 if toggled. |
| `cui_icon_button(ctx, id, icon)` | Icon button (CUI_ICON_*); returns 1 if clicked. |
| `cui_text_input(ctx, id, buffer, capacity, opts)` | Text input; returns 1 if buffer changed. |
| `cui_spacer(ctx, w, h)` | Fixed-size spacer. |
| `cui_center(ctx)` | Center children. |
| `cui_row(ctx, opts)` | Row layout. |
| `cui_column(ctx, opts)` | Column layout. |
| `cui_stack(ctx, opts)` | Stack (overlapping). |
| `cui_wrap(ctx, opts)` | Wrap layout. |
| `cui_scroll(ctx, id, opts)` | Scroll container; id may be NULL. |
| `cui_end(ctx)` | Close last container. |
| `cui_canvas(ctx, opts)` | Custom-draw surface; use cui_draw_* inside; close with cui_end. |
| `cui_draw_rect(ctx, x, y, w, h, color)` | Rectangle (0xAARRGGBB). |
| `cui_draw_circle(ctx, cx, cy, r, color)` | Circle. |
| `cui_draw_text(ctx, x, y, text, color)` | Text. |

## Input injection

| API | Description |
|-----|-------------|
| `cui_inject_click(ctx, x, y)` | Mouse click. |
| `cui_inject_mouse_move(ctx, x, y)` | Pointer position (hover, scroll target). |
| `cui_inject_scroll(ctx, dx, dy)` | Scroll wheel; dy &lt; 0 = down. |
| `cui_inject_key(ctx, key)` | Key (CUI_KEY_TAB, CUI_KEY_ENTER, etc.). |
| `cui_inject_char(ctx, codepoint)` | Printable character into focused text input. |

## Queries

| API | Description |
|-----|-------------|
| `cui_ctx_hovered_id(ctx)` | Id of widget under pointer, or NULL. |
| `cui_ctx_is_hovered(ctx, id)` | 1 if id is hovered. |

## Accessibility

| API | Description |
|-----|-------------|
| `cui_aria_label(ctx, label)` | Override for next widget. |
| `cui_tab_index(ctx, index)` | Tab order for next widget. |

## Debug (CUI_DEBUG only)

| API | Description |
|-----|-------------|
| `cui_dev_overlay(ctx)` | Frame time, node count, arena usage. |

## Platform (clearui_platform.h)

| API | Description |
|-----|-------------|
| `cui_platform_stub_get(void)` | Built-in headless platform. |

`cui_platform` struct: window_create, window_destroy, window_get_size, poll_events, clipboard_get/set, cursor_set, scale_factor_get, surface_get/destroy, present_software.

## RDI (clearui_rdi.h)

| API | Description |
|-----|-------------|
| `cui_rdi_soft_get(void)` | Built-in software RDI. |
| `cui_rdi_soft_set_viewport(ctx, w, h)` | Set framebuffer size. |
| `cui_rdi_soft_set_platform(ctx, platform_ctx, platform)` | Wire present_software. |
| `cui_rdi_soft_get_framebuffer(ctx, &rgba, &w, &h)` | Get framebuffer for tests. |

`cui_rdi` struct: init, shutdown, texture_create/upload/destroy, submit, scissor, present.
