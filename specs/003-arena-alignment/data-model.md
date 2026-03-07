# Data Model: Arena Alignment

**Branch**: `003-arena-alignment` | **Date**: 2026-03-07

## Modified Entity: `cui_arena`

No structural change to the `cui_arena` struct. The fields (`base`, `pos`, `cap`) remain identical.

### Behavioral Change

| Function | Before | After |
|----------|--------|-------|
| `cui_arena_alloc(a, size)` | Returns `base + pos`; bumps `pos += size` | Pads `pos` to `_Alignof(max_align_t)` boundary, then returns `base + pos`; bumps `pos += size` |
| `cui_arena_alloc_aligned(a, size, align)` | Calls `cui_arena_alloc` for padding bytes | Directly bumps `pos` for padding (avoids double-alignment overhead) |

### Invariants (new)

- Every pointer returned by `cui_arena_alloc` satisfies `(uintptr_t)ptr % _Alignof(max_align_t) == 0`
- Every pointer returned by `cui_arena_alloc_aligned` satisfies `(uintptr_t)ptr % align == 0`
- `a->pos` after any allocation is NOT guaranteed to be aligned (only the returned pointer is)
