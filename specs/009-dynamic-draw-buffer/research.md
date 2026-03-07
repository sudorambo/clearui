# Research: Dynamic Draw Command Buffer / Configurable Capacity

## Decision 1: Configurable capacity via `cui_config`

**Decision**: Add a field to `cui_config` (e.g. `draw_buf_capacity` or `max_draw_commands`) so the application can set the buffer size at context creation. Use `0` to mean "use default" (1024) for backward compatibility.

**Rationale**: Keeps the public API minimal (no new init functions), satisfies Constitution API surface cap, and makes the choice explicit at create time. Callers that never set the field get current behavior.

**Alternatives considered**:
- Separate `cui_create_ex(config, draw_buf_capacity)`: Rejected to avoid API growth; config struct is the standard extension point.
- Environment variable or compile-time only: Rejected; spec requires runtime configurable capacity.

---

## Decision 2: Buffer representation (fixed array vs heap-allocated)

**Decision**: Change `cui_draw_command_buffer` to hold a pointer to the command array plus `capacity` and `count`. Context allocates the array(s) in `cui_create` using `cui_config.draw_buf_capacity` (or default 1024) and frees in `cui_destroy`. No flexible array member in a shared header that RDI sees—RDI continues to receive a pointer to a buffer with `cmd`, `count`, and `capacity` (or equivalent) so it can iterate without knowing allocation details.

**Rationale**: C11 does not allow variable-length arrays in structs with stable ABI; a pointer allows configurable size. Ownership stays with the context; no hidden allocations (alloc at create, free at destroy). RDI contract can remain "opaque buffer with count and command array" so existing RDI implementations still work if they use `count` and the same `cui_draw_cmd` layout.

**Alternatives considered**:
- Keep fixed array and allow only compile-time override: Rejected; spec requires runtime configurable capacity.
- Two structs (internal with pointer vs external with fixed array): Rejected for complexity; single struct with pointer and capacity is simpler and RDI can still take `const cui_draw_command_buffer *`.

---

## Decision 3: Default capacity and overflow behavior

**Decision**: Default capacity is 1024 when `draw_buf_capacity` is 0 or not set. When capacity is exceeded, push functions return -1 and do not grow the buffer (no silent drop without return value). Optional dynamic growth can be a follow-up (e.g. config flag + realloc in push).

**Rationale**: Backward compatibility and predictable behavior. Callers can check return value; no hidden realloc in hot path by default. Matches spec R2 (defined behavior: fail push or grow; callers can detect failure).

**Alternatives considered**:
- Silent truncation: Rejected; spec requires callers able to detect failure.
- Always dynamic growth: Rejected for first iteration; adds realloc in hot path and may conflict with embedded/safety-critical use cases that want a fixed cap.

---

## Decision 4: RDI and render path

**Decision**: `cui_draw_command_buffer` will have `cui_draw_cmd *cmd` (or similar) and `size_t capacity` in addition to `size_t count`. Render path (`scale_buf`) and RDI submit iterate using `count` and the same command layout; they do not use a `CUI_DRAW_BUF_MAX` macro. Include/clearui_rdi.h may forward-declare the buffer type; internal layout (pointer vs array) is in draw_cmd.h so RDI drivers see the same access pattern (buffer->cmd[i], buffer->count).

**Rationale**: Minimal change to RDI contract; buffer is still a contiguous array of `cui_draw_cmd`. Only allocation and capacity check change.

**Alternatives considered**:
- Keeping a macro for render loop upper bound: Rejected; loop must use buffer->count (and optionally buffer->capacity for bounds) so that variable-size buffers work.
