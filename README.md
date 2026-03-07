# ClearUI

A C11 GUI library with a **declarative immediate** model. You write immediate-mode code each frame; the library maintains an internal retained tree for layout, diffing, accessibility, and rendering.

**Zero dependencies.** Compiles with `gcc`, `clang`, or MSVC. No CMake, no pkg-config, no vcpkg.

- **Color format**: All colors are 32-bit `0xAARRGGBB` (alpha, red, green, blue; alpha in high byte).
- **Single-threaded**: All API calls must happen on one thread; no locking; multi-threaded use is undefined.
- **Fixed limits** (silent truncation when exceeded): container nesting 16, focusables per frame 64, a11y entries 128, widget ID length 64 bytes, `cui_frame_printf` output 64 KiB. See `include/clearui.h` for macro names.

## Hello World

```c
#include "clearui.h"

int main(void) {
    cui_ctx *ctx = cui_create(&(cui_config){
        .title = "Hello", .width = 400, .height = 300,
    });

    while (cui_running(ctx)) {
        cui_begin_frame(ctx);
        cui_center(ctx);
            cui_label(ctx, "Hello, World!");
        cui_end(ctx);
        cui_end_frame(ctx);
    }

    cui_destroy(ctx);
}
```

## Counter (state + buttons)

```c
#include "clearui.h"

int main(void) {
    cui_ctx *ctx = cui_create(&(cui_config){
        .title = "Counter", .width = 320, .height = 200,
    });

    while (cui_running(ctx)) {
        cui_begin_frame(ctx);

        int *count = cui_state(ctx, "counter", sizeof(int));
        cui_center(ctx);
            cui_column(ctx, &(cui_layout){ .align = CUI_ALIGN_CENTER, .gap = 12 });
                cui_label(ctx, cui_frame_printf(ctx, "Count: %d", *count));
                cui_row(ctx, &(cui_layout){ .gap = 8 });
                    if (cui_button(ctx, "-")) (*count)--;
                    if (cui_button(ctx, "+")) (*count)++;
                cui_end(ctx);
            cui_end(ctx);
        cui_end(ctx);

        cui_end_frame(ctx);
    }

    cui_destroy(ctx);
}
```

`cui_state` gives you persistent, zero-initialized storage keyed by a string. `cui_frame_printf` returns a string that lives until the next frame.

## Full API Demo

[`examples/demo.c`](examples/demo.c) exercises every public API function in a single commented file. It runs headlessly and prints a summary of what happened:

```bash
make demo && ./demo
```

Read it as a reference for layout, widgets, state, styling, canvas, accessibility, keyboard input, and platform/RDI wiring.

## Build

Requires a C11 compiler. Nothing else.

```bash
make all                # compile all objects
make lib                # build libclearui.a
make unit-tests         # run 10 unit tests
make integration-tests  # run 3 integration tests
make clean              # remove artifacts
```

To verify everything works:

```bash
make clean && make all && make unit-tests && make integration-tests && make lib
```

## Project Structure

```
include/
  clearui.h              # public API (~60 functions)
  clearui_platform.h     # platform adapter interface
  clearui_rdi.h          # render driver interface

examples/
  demo.c                 # full API reference (make demo && ./demo)

src/
  core/                  # context, arena, vault, diff, draw commands, a11y
  layout/                # flexbox-style layout engine
  font/                  # text measurement (vendored stb_truetype)
  widget/                # label, button, checkbox, scroll, canvas, ...
  platform/              # headless stub (swap for SDL3, GLFW, native)
  rdi/                   # software RDI stub (swap for Vulkan, Metal, WebGPU)

tests/
  unit/                  # 10 tests: arena, vault, layout, font, draw_buf,
                         #           diff, frame_alloc, draw_cmd, a11y, focus
  integration/           # 3 tests:  hello, counter, rdi_platform
```

## Architecture

```
 Application
     |
 Widget layer     cui_button, cui_label, cui_row, cui_scroll, ...
     |
 Layout layer     flexbox-style: row, column, center, stack, wrap
     |
 Core layer       context, arena, vault, diff engine, draw commands
     |
 Platform / RDI   window, events, rendering (all behind interfaces)
```

Dependencies flow strictly downward. Application code uses Widget and Layout; Core is for custom widgets; Platform/RDI are behind function-pointer interfaces.

### Memory model

| Allocator | Lifetime | Use |
|-----------|----------|-----|
| **Arena** | Reset each frame | UI tree nodes |
| **Frame allocator** | Reset each frame | Transient strings (`cui_frame_printf`) |
| **Vault** | Persistent | Application state (`cui_state`) |

You never call `free()` on UI nodes. The arena handles it.

## Widgets

| Widget | Function | Returns |
|--------|----------|---------|
| Label | `cui_label(ctx, "text")` | void |
| Styled label | `cui_label_styled(ctx, "text", &style)` | void |
| Button | `cui_button(ctx, "id")` | 1 if clicked |
| Checkbox | `cui_checkbox(ctx, "id", &checked)` | 1 if toggled |
| Text input | `cui_text_input(ctx, "id", buf, cap, &opts)` | 1 if changed |
| Icon button | `cui_icon_button(ctx, "id", CUI_ICON_TRASH)` | 1 if clicked |
| Spacer | `cui_spacer(ctx, w, h)` | void |

## Layout containers

All containers are closed with `cui_end(ctx)`.

```c
cui_center(ctx);                                         // center children
cui_row(ctx, &(cui_layout){ .gap = 8 });                 // horizontal
cui_column(ctx, &(cui_layout){ .gap = 12, .padding = 16 }); // vertical
cui_stack(ctx, &(cui_layout){});                          // overlapping
cui_wrap(ctx, &(cui_layout){ .gap = 4 });                // flow wrap
cui_scroll(ctx, &(cui_scroll_opts){ .max_height = 200 }); // scrollable
```

### Layout options

```c
typedef struct cui_layout {
    float gap;                            // space between children
    float padding, padding_x, padding_y;  // inner padding
    float min_width, max_width;           // size constraints
    float min_height, max_height;
    float flex;                           // flex grow factor
    int   align;    // CUI_ALIGN_START | CENTER | END | STRETCH
    int   align_y;  // cross-axis alignment
} cui_layout;
```

## Canvas (custom drawing)

```c
cui_canvas(ctx, &(cui_canvas_opts){ .width = 200, .height = 200 });
    cui_draw_rect(ctx, 10, 10, 50, 50, 0xff0000ff);
    cui_draw_circle(ctx, 100, 100, 30, 0xff00ff00);
    cui_draw_text(ctx, 10, 180, "hello", 0xff000000);
cui_end(ctx);
```

## Styling

```c
cui_push_style(ctx, &(cui_style){ .text_color = 0xff0000ff });
    cui_label(ctx, "this is red");
cui_pop_style(ctx);
```

## Keyboard & Accessibility

```c
cui_tab_index(ctx, 1);                   // explicit tab order for next widget
cui_aria_label(ctx, "Close dialog");     // screen reader label for next widget
cui_inject_key(ctx, CUI_KEY_TAB);        // programmatic focus control
```

For the focused text input, use `cui_inject_char(ctx, codepoint)` to insert printable characters (e.g. ASCII 32–126) at the cursor, and `cui_inject_key(ctx, CUI_KEY_BACKSPACE)` or `cui_inject_key(ctx, CUI_KEY_DELETE)` to remove the character before or at the cursor. `cui_text_input` returns **1** when that widget’s buffer was modified in the previous frame (insert or backspace/delete), **0** otherwise.

Focus indicators follow WCAG 2.1 AA. The internal accessibility tree exposes roles, labels, and composable state (`"checked"`, `"focused"`, `"checked focused"`) for platform screen reader integration.

## Platform & Rendering

ClearUI separates platform (windowing/events) and rendering into two interface structs:

```c
#include "clearui_platform.h"  // cui_platform — window, events, clipboard
#include "clearui_rdi.h"       // cui_rdi — init, submit, present, textures

cui_set_platform(ctx, my_platform, my_platform_ctx);
cui_set_rdi(ctx, my_rdi, my_rdi_ctx);
```

Built-in stubs for headless testing:

```c
const cui_platform *plat = cui_platform_stub_get();
const cui_rdi *rdi = cui_rdi_soft_get();
```

## Using the static library

```bash
make lib   # produces libclearui.a

# link against it (bring your own platform + RDI)
cc main.c -I include -L . -lclearui -lm -o myapp
```

`libclearui.a` includes core, font, layout, and widgets. Platform and RDI stubs are excluded so you can provide your own.

## CI

GitHub Actions runs `make all`, `make unit-tests`, and `make integration-tests` on Ubuntu, macOS, and Windows on every push/PR. A separate job runs tests under AddressSanitizer and UndefinedBehaviorSanitizer on Ubuntu. See [.github/workflows/ci.yml](.github/workflows/ci.yml).

## Contributing

The codebase compiles with `-std=c11 -Wall -Wextra -Wpedantic` and zero warnings. All PRs must maintain this.

```bash
# before submitting
make clean && make all && make unit-tests && make integration-tests
```

Optional: run tests under AddressSanitizer and UndefinedBehaviorSanitizer:

```bash
make asan    # build and run tests with ASan
make ubsan   # build and run tests with UBSan
```

Format C sources with the project style:

```bash
clang-format -i path/to/file.c
# or format all: find src tests examples -name '*.c' -exec clang-format -i {} \;
```

The repo root contains `.clang-format`; use it for consistent style.

Design documents and feature specs live under `specs/`. The project constitution is at `.specify/memory/constitution.md`.

## License

Licensed under either of

- [Apache License, Version 2.0](LICENSE-APACHE)
- [MIT License](LICENSE-MIT)

at your option.

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in this project by you shall be dual-licensed as above, without any additional terms or conditions.
