# Quickstart: ClearUI

Get from zero to a window with a button in under five minutes. No CMake, no pkg-config.

## Prerequisites

- C11 compiler: `gcc`, `clang`, or MSVC.
- ClearUI: two-file drop-in (`clearui.h` + `clearui.c`) or precompiled static library.
- One platform adapter: e.g. SDL3 (recommended). Install SDL3 per your OS.
- One RDI: e.g. software backend for first run (`clearui_rdi_soft`) or Vulkan/Metal/WebGPU driver.

## 1. Hello World

Create `main.c`:

```c
#include "clearui.h"

int main(void)
{
    cui_ctx *ctx = cui_create(&(cui_config){
        .title  = "Hello ClearUI",
        .width  = 400,
        .height = 300,
    });

    while (cui_running(ctx)) {
        cui_begin_frame(ctx);

        cui_center(ctx);
            cui_label(ctx, "Hello, World!");
        cui_end(ctx);

        cui_end_frame(ctx);
    }

    cui_destroy(ctx);
    return 0;
}
```

## 2. Build (single command)

**From ClearUI repo root (no SDL3):** build and run Hello World / Counter with the stub platform:

```bash
make test_hello && ./test_hello
make test_counter && ./test_counter
```

**With amalgamation (when available) or precompiled ClearUI and SDL3:**

```bash
cc main.c clearui.c -I. -lSDL3 -lm -o hello
# or: cc main.c -lclearui -lSDL3 -lm -o hello
```

**Windows (MSVC):**

```cmd
cl main.c clearui.lib /I. SDL3.lib /Fe:hello.exe
```

## 3. Run

```bash
./hello
```

You should see a window titled "Hello ClearUI" with centered text "Hello, World!".

## 4. Add a button and state

```c
#include "clearui.h"

int main(void)
{
    cui_ctx *ctx = cui_create(&(cui_config){
        .title = "Counter", .width = 320, .height = 200,
    });

    while (cui_running(ctx)) {
        cui_begin_frame(ctx);

        int *count = cui_state(ctx, "counter", sizeof(int));
        cui_center(ctx);
            cui_column(ctx, &(cui_layout){ .align = CUI_ALIGN_CENTER, .gap = 12 });
                cui_label(ctx, cui_frame_printf(ctx, "Count: %d", *count));
                if (cui_button(ctx, "+ Increment")) (*count)++;
            cui_end(ctx);
        cui_end(ctx);

        cui_end_frame(ctx);
    }

    cui_destroy(ctx);
    return 0;
}
```

Rebuild and run. Click the button; the count increments. State survives frame-to-frame via `cui_state`.

## 5. Next steps

- **Layout**: Use `cui_row`, `cui_column`, `cui_layout` (gap, padding, align). See spec §6.
- **Widgets**: `cui_text_input`, `cui_checkbox`, `cui_scroll`, `cui_canvas`. See spec §5.
- **Style**: `cui_push_style` / `cui_pop_style` for colors and text style.
- **Distribution**: Copy `clearui.h` and `clearui.c` into your project, or link the static library and include only `clearui.h`.

## Troubleshooting

- **No window**: Ensure platform adapter (e.g. SDL3) is linked and initialized; check that `cui_create` receives a valid config and platform/RDI if required by your build.
- **No render**: Ensure an RDI is selected (e.g. `CUI_BACKEND_SOFT` or Vulkan) and that the driver init succeeds.
- **Build errors**: Use `-std=c11 -Wall -Wextra`; include the directory containing `clearui.h`.

End of quickstart.
