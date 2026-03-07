# Data Model: Draw Command Buffer Configuration

## Entities

### cui_config (extended)

| Field               | Type   | Description |
|---------------------|--------|-------------|
| title               | const char * | (existing) |
| width, height       | int    | (existing) |
| scale_factor        | float  | (existing) |
| **draw_buf_capacity** | size_t | **New.** Max number of draw commands per buffer. If 0, use default 1024. Set before `cui_create`. |

- Validation: If `draw_buf_capacity > 0`, context allocates buffers of that size. No enforced maximum (caller responsibility); reasonable upper bound can be documented (e.g. 65536).

### cui_draw_command_buffer (internal / draw_cmd.h)

| Field     | Type           | Description |
|-----------|----------------|-------------|
| cmd       | cui_draw_cmd * | Pointer to heap-allocated array of commands. Owned by context. |
| capacity  | size_t         | Allocated length of `cmd`. |
| count     | size_t         | Number of commands currently in buffer (0 ≤ count ≤ capacity). |

- Lifecycle: Allocated in `cui_create` (or buffer init helper called from context); freed in `cui_destroy` (or buffer fini helper). No ownership transfer to RDI (RDI receives const pointer for submit).

### cui_ctx (context.c)

- Holds two buffers: `draw_buf`, `canvas_cmd_buf`. Both are `cui_draw_command_buffer` with their own `cmd` pointer and `capacity`. Same capacity can be used for both, from `cui_config.draw_buf_capacity` (default 1024).

## State transitions

- **Context create**: For each buffer, allocate `cmd` with `capacity = config.draw_buf_capacity ? config.draw_buf_capacity : 1024`. Set `count = 0`.
- **Frame (begin/end)**: `cui_draw_buf_clear` sets `count = 0`; no reallocation.
- **Push**: If `count < capacity`, append command and increment `count`, return 0. Else return -1.
- **Context destroy**: Free each buffer’s `cmd`; set pointer to NULL, capacity/count to 0.

## Relationships

- One `cui_config` per `cui_create` call; config is read only at create time.
- One `cui_ctx` owns two `cui_draw_command_buffer` instances (main draw_buf, canvas_cmd_buf).
- RDI and render path read `buffer->count` and `buffer->cmd`; they do not modify capacity or reallocate.
