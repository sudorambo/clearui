# ClearUI

A C11 GUI library with a **declarative immediate** model. You write immediate-mode code each frame; the library maintains an internal retained tree for layout, diffing, accessibility, and rendering.

**Zero dependencies.** Compiles with `gcc`, `clang`, or MSVC. Ships a Makefile and an optional CMakeLists.txt (with `find_package` and pkg-config support).

- **Color format**: All colors are 32-bit `0xAARRGGBB` (alpha, red, green, blue; alpha in high byte).
- **Single-threaded**: All API calls must happen on one thread; no locking; multi-threaded use is undefined.
- **Fixed limits** (silent truncation when exceeded unless you set an error callback): container nesting 16, focusables per frame 64, a11y entries 128, widget ID length 64 bytes, `cui_frame_printf` output 64 KiB. Set `cui_config.error_callback` and `error_userdata` to be notified when a limit is hit. Define **`CUI_DEBUG`** when building for development to enable assertions (e.g. unbalanced push/pop, parent stack overflow). See `include/clearui.h` for macro names and error codes.

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
make lib                # build libclearui.a (core only; no RDI/platform)
make overlay            # build libclearui_overlay.a (core + soft RDI + platform stub)
make shaders            # recompile SPIR-V from GLSL (requires glslangValidator or glslc)
make embed-font         # generate embedded font header (src/font/default_font_embed.h)
make unit-tests         # run 24 unit tests
make integration-tests  # run 5 integration tests
make stress             # stress test (1000+ widgets)
make leak-check         # unit tests under Valgrind (Linux); only "definitely lost" fails
make leak-check-lsan    # leak check via LeakSanitizer (use if Valgrind fails at startup)
make fuzz-utf8          # build UTF-8 fuzz target (libFuzzer; needs clang)
make fuzz-vault         # build vault fuzz harness (standalone)
make fuzz-frame         # build frame allocator fuzz harness (standalone)
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
  clearui_overlay_spv.h  # pre-compiled overlay SPIR-V as C byte arrays

shaders/
  overlay.vert           # fullscreen triangle vertex shader (GLSL 450)
  overlay.frag           # overlay fragment shader with RGBA/BGRA toggle
  overlay.vert.spv       # pre-compiled SPIR-V
  overlay.frag.spv       # pre-compiled SPIR-V

docs/
  API.md                 # API reference (list of public functions)
  MIGRATION.md           # upgrade notes 0.2.0 → 1.1.0

examples/
  demo.c                 # full API reference (make demo && ./demo)
  vulkan_overlay.c       # Vulkan overlay integration reference

src/
  core/                  # context, arena, vault, diff, draw commands, a11y
  layout/                # flexbox-style layout engine
  font/                  # text measurement (vendored stb_truetype)
  widget/                # label, button, checkbox, scroll, canvas, ...
  platform/              # headless stub (swap for SDL3, GLFW, native)
  rdi/                   # software RDI (rasterizes to RGBA; optional GPU backends later)

tests/
  unit/                  # 24 tests: arena, vault, layout, font, utf8, draw_buf, diff,
                         #           frame_alloc, draw_cmd, a11y, focus, text_input,
                         #           scroll, canvas_draw, label_styled, spacer, wrap,
                         #           stack, style_stack, cui_frame_alloc, scale_buf, edge_cases, theme, rdi_soft
  fuzz/                  # utf8_fuzz (libFuzzer)
  stress/                # stress_widgets (1000+ widgets)
  integration/           # 5 tests:  hello, counter, rdi_platform, text_input_edit, scroll_region

CMakeLists.txt           # CMake build (clearui + clearui_overlay targets, install, pkg-config)
clearui.pc.in            # pkg-config template (generated by CMake install)
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
cui_scroll(ctx, NULL, &(cui_scroll_opts){ .max_height = 200 }); // scrollable (id NULL = no wheel target)
```

### Layout options

```c
typedef struct cui_layout {
    float gap;                            // space between children
    float padding, padding_x, padding_y;   // inner padding: padding_x/y override padding when > 0
    float min_width, max_width;           // size constraints
    float min_height, max_height;
    float flex;                           // flex grow factor
    int   align;    // CUI_ALIGN_START | CENTER | END | STRETCH
    int   align_y;  // cross-axis alignment
} cui_layout;
```

Use `padding` for uniform padding, or `padding_x` / `padding_y` for per-axis; explicit `padding_x` or `padding_y` override `padding` when greater than 0.

## Canvas (custom drawing)

```c
cui_canvas(ctx, &(cui_canvas_opts){ .width = 200, .height = 200 });
    cui_draw_rect(ctx, 10, 10, 50, 50, 0xff0000ff);
    cui_draw_circle(ctx, 100, 100, 30, 0xff00ff00);
    cui_draw_text(ctx, 10, 180, "hello", 0xff000000);
cui_end(ctx);
```

## Theming

The default look (colors, corner radius, font size, focus ring) is stored in a **runtime theme**. Set it with `cui_set_theme(ctx, &theme)`; pass `NULL` to reset to the built-in default. The library copies the struct, so you can use stack or static storage.

```c
cui_theme t;
t.text_color = 0xff000000u;
t.button_bg = 0xffe0e0e0u;
t.checkbox_bg = 0xffffffffu;
t.input_bg = 0xffffffffu;
t.corner_radius = 4.f;
t.font_size = 16;
t.focus_ring_color = 0xff0066ccu;
t.focus_ring_width = 2.f;
cui_set_theme(ctx, &t);
```

For a dark preset, call `cui_theme_dark(&t)` to fill `t`, then `cui_set_theme(ctx, &t)`.

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

**Scroll and hover**: Call `cui_inject_mouse_move(ctx, x, y)` each frame (or when the platform reports motion). The scroll container **under the pointer** receives scroll wheel events: use `cui_inject_scroll(ctx, dx, dy)` (e.g. `dy < 0` = scroll down). Give scroll containers an id so they can be targeted: `cui_scroll(ctx, "panel", &(cui_scroll_opts){ .max_height = 200 })`. Use `cui_ctx_hovered_id(ctx)` or `cui_ctx_is_hovered(ctx, "btn")` to highlight the widget under the pointer. Optional: when `cui_ctx_hovered_id` changes, call `platform->cursor_set(platform_ctx, shape)` (see `clearui_platform.h`) to change the cursor shape (e.g. pointer over buttons).

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

**Bring your own platform**: Implement the `cui_platform` struct (see `include/clearui_platform.h`). **Required**: `window_create`, `window_destroy`, `window_get_size`, `poll_events`. **Optional** (set to NULL if not supported): `clipboard_get`, `clipboard_set`, `cursor_set`, `scale_factor_get`, `surface_get`, `surface_destroy`, `present_software` (for software RDI blit). The second argument to `poll_events` is `cui_ctx*` (passed as `void*`); use it to call `cui_inject_mouse_move`, `cui_inject_click`, `cui_inject_scroll`, `cui_inject_key`, `cui_inject_char` as events occur. Return `false` from `poll_events` when the app should quit. Call `cui_set_platform(ctx, &my_platform, my_platform_ctx)` before the first `cui_begin_frame`. See `specs/020-platform-backend-0-7-0/contracts/platform-adapter-0.7.md` and `quickstart.md` in that spec for details.

**SDL3 adapter** (optional): Build with `WITH_SDL3=1` when SDL3 is installed (`sdl3-config` or `pkg-config SDL3`). Then link your app with `cui_platform_sdl3.o` and SDL3. The integration test `test_platform_window` opens a real window, runs one frame, and closes; it is skipped in headless CI when no display is available.

**Software RDI**: The built-in `cui_rdi_soft_get()` rasterizes the draw command buffer to an RGBA framebuffer (rects, rounded rects, lines, text via stb_truetype). Call `cui_rdi_soft_set_viewport(rdi_ctx, width, height)` before each frame (e.g. from `platform->window_get_size`) so the framebuffer is allocated. To display on screen, use a platform that implements `present_software` and call `cui_rdi_soft_set_platform(rdi_ctx, platform_ctx, &platform)` so that `present()` blits the framebuffer to the window. See `specs/021-render-driver-0-8-0/quickstart.md`.

## Using the static library

```bash
make lib       # produces libclearui.a  (core only; bring your own platform + RDI)
make overlay   # produces libclearui_overlay.a (core + soft RDI + platform stub)

# link against the core library (bring your own platform + RDI)
cc main.c -I include -L . -lclearui -lm -o myapp

# or link the overlay library (everything included)
cc main.c -I include -L . -lclearui_overlay -lm -o myapp
```

`libclearui.a` includes core, font, layout, and widgets. Platform and RDI stubs are excluded so you can provide your own. `libclearui_overlay.a` adds the software RDI and headless platform stub for consumers who just want to render to an RGBA framebuffer (e.g. Vulkan/OpenGL overlay).

## Font configuration

The default font is loaded from `deps/default_font.ttf` relative to the working directory. For projects where this path does not apply, two runtime alternatives are available:

```c
cui_set_font_path("/absolute/path/to/my_font.ttf");
```

Or load from memory (no file I/O at all):

```c
cui_load_font_memory(ttf_data, ttf_len);
```

Both must be called before the first `cui_begin_frame`.

For single-binary distribution, build with `-DCLEARUI_EMBED_FONT` to embed the default TTF as a C byte array. Generate the header first with `make embed-font`.

## Vulkan overlay integration

ClearUI renders to an RGBA framebuffer via the software RDI. To composite this onto a Vulkan scene:

1. Each frame, call `cui_rdi_soft_get_framebuffer(rdi_ctx, &rgba, &w, &h)` to obtain the pixel buffer.
2. Upload it to a Vulkan texture.
3. Draw a fullscreen triangle using the shipped overlay shaders.

Pre-compiled SPIR-V is available in `shaders/overlay.vert.spv` and `shaders/overlay.frag.spv`, and as C byte arrays in `include/clearui_overlay_spv.h` for embedding without a shader compiler. The fragment shader has a push-constant toggle for RGBA-to-BGRA swizzle (set `swizzle_rb = 1` for BGRA swapchains).

See [`examples/vulkan_overlay.c`](examples/vulkan_overlay.c) for a complete annotated reference.

## CMake build

An optional `CMakeLists.txt` is provided for projects using CMake:

```bash
cmake -B build
cmake --build build
cmake --install build --prefix /usr/local
```

This produces `libclearui.a` and `libclearui_overlay.a`, installs public headers, shaders, and generates both a CMake config file (`find_package(ClearUI)`) and a `clearui.pc` for pkg-config.

Options: `-DCLEARUI_EMBED_FONT=ON` to embed the default font, `-DCLEARUI_BUILD_EXAMPLES=ON` to build examples.

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

**API freeze**: The public API is frozen as of 1.0.0. New APIs are additive only; no breaking changes until 2.0.

## License

Licensed under either of

- [Apache License, Version 2.0](LICENSE-APACHE)
- [MIT License](LICENSE-MIT)

at your option.

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in this project by you shall be dual-licensed as above, without any additional terms or conditions.
