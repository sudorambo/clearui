# Quickstart: Configurable Draw Buffer Capacity

## Use case

Your UI produces more than 1024 draw commands per frame (e.g. large list, many canvas primitives). Without changing the library source, you want to increase the draw buffer size so no commands are dropped.

## Steps

1. **Set capacity at create time**

   When creating the context, set `draw_buf_capacity` in the config. Use 0 to keep the default (1024).

   ```c
   #include "clearui.h"

   int main(void) {
       cui_config config = {
           .title = "My App",
           .width = 800,
           .height = 600,
           .scale_factor = 1.f,
           .draw_buf_capacity = 4096,   /* allow up to 4096 commands per buffer */
       };
       cui_ctx *ctx = cui_create(&config);
       if (!ctx) { /* handle alloc failure */ return 1; }
       /* ... run UI loop ... */
       cui_destroy(ctx);
       return 0;
   }
   ```

2. **Optional: handle push failure**

   If you do not set a large enough capacity and the buffer fills, `cui_draw_buf_push_*` returns -1. The internal build-from-tree path may not check this today; a future improvement is to either grow the buffer (if enabled) or surface an error. For now, setting a sufficiently large `draw_buf_capacity` avoids overflow.

3. **Default behavior**

   If you never set `draw_buf_capacity` (or set it to 0), behavior is unchanged: capacity 1024, same as before the feature.

## Integration points

- **Config**: `include/clearui.h` — `cui_config.draw_buf_capacity`
- **Context**: `cui_create` reads the field and allocates buffers; `cui_destroy` frees them.
- **RDI**: No change; RDI still receives the same buffer type and iterates by `count` and `cmd`.
