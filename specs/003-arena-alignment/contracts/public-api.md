# Contract: Arena Allocator Public API

**Branch**: `003-arena-alignment` | **Date**: 2026-03-07

## Interface: `cui_arena_alloc`

**Header**: `src/core/arena.h`
**Signature**: `void *cui_arena_alloc(cui_arena *a, size_t size);`

### Preconditions

- `a` is a valid, initialized `cui_arena` (via `cui_arena_init`)
- `size > 0` (size 0 returns `NULL`)

### Postconditions

- Returns a pointer aligned to at least `_Alignof(max_align_t)` (typically 8 or 16 bytes)
- Returned memory is uninitialized
- `a->pos` is advanced by at least `size` bytes (plus alignment padding)
- On failure (arena full and `grow` fails): returns `NULL`

### Guarantee (new)

```
For every non-NULL return value p:
    (uintptr_t)p % _Alignof(max_align_t) == 0
```

This matches the alignment guarantee of `malloc`.

### Unchanged

- Function signature: no change
- ABI: no change (struct layout, calling convention identical)
- Thread safety: none (same as before — single-threaded use only)

---

## Interface: `cui_arena_alloc_aligned`

**Header**: `src/core/arena.h`
**Signature**: `void *cui_arena_alloc_aligned(cui_arena *a, size_t size, size_t align);`

### Preconditions

- Same as `cui_arena_alloc`, plus:
- `align` must be a power of 2

### Postconditions

- Returns a pointer aligned to `align` bytes (using absolute-address alignment, not position-relative)
- Works correctly for alignments larger than `_Alignof(max_align_t)` (e.g. 64-byte cache-line alignment)
- Recalculates padding after potential realloc in grow path

### Unchanged

- Function signature: no change
- ABI: no change
