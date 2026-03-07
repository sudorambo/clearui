# Contract: Draw Buffer Configuration

## cui_config.draw_buf_capacity

- **Type**: `size_t`
- **When read**: Only at `cui_create`. Not read after context creation.
- **Semantics**:
  - `0`: Use default capacity 1024.
  - `> 0`: Use this value as the maximum number of draw commands per buffer (main and canvas). Context allocates buffers of this size.
- **Guarantees**: No allocation of draw buffers before `cui_create`; allocation happens once per buffer inside `cui_create`. No reallocation during frame.

## Buffer capacity and overflow

- **Push functions** (`cui_draw_buf_push_rect`, `cui_draw_buf_push_line`, `cui_draw_buf_push_text`): Return `0` on success. Return `-1` when buffer is full (`count >= capacity`) and no growth is performed; the command is not appended.
- **Callers**: Internal build-from-tree code and canvas widget push into the buffer; they may ignore the return value today. Applications that use a custom capacity should ensure it is large enough, or future work can add error handling when push returns -1.
