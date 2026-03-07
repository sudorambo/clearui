# Research: Add Default Alignment to Arena Allocator

**Branch**: `003-arena-alignment` | **Date**: 2026-03-07

## Research Questions

### 1. What alignment guarantee should `cui_arena_alloc` provide?

- **Decision**: Align to `_Alignof(max_align_t)`.
- **Rationale**: `max_align_t` (defined in `<stddef.h>`, C11) has the alignment of the most-aligned scalar type on the platform. `_Alignof(max_align_t)` is typically 8 on LP64 (most 64-bit Unix) and 16 on some platforms (x86-64 with SSE, macOS). This matches what `malloc` guarantees — any type can be safely stored in the returned memory.
- **Alternatives considered**:
  - `sizeof(void*)`: Sufficient for pointers but not for `long double` or SIMD types on some platforms. Weaker than the `malloc` guarantee.
  - Hardcoded 8 or 16: Non-portable. Would need `#if` platform detection.
  - `alignof(cui_node)`: Too specific. The arena is general-purpose and may allocate types other than `cui_node` in the future.

### 2. How should alignment be implemented?

- **Decision**: Compute alignment padding inline within `cui_arena_alloc`, before the bump.
- **Rationale**: This is the standard arena pattern. Padding is `(align - (pos % align)) % align`, added to `pos` before the allocation. The existing `cui_arena_alloc_aligned` already uses this formula; the change is to apply it by default in `cui_arena_alloc` with a fixed alignment constant.
- **Implementation approach**:

  ```c
  #define CUI_ARENA_DEFAULT_ALIGN _Alignof(max_align_t)

  void *cui_arena_alloc(cui_arena *a, size_t size) {
      if (size == 0) return NULL;
      size_t align = CUI_ARENA_DEFAULT_ALIGN;
      size_t pad = (align - (a->pos % align)) % align;
      size_t total = pad + size;
      if (a->pos + total > a->cap) grow(a, total);
      if (a->pos + total > a->cap) return NULL;
      a->pos += pad;
      void *p = a->base + a->pos;
      a->pos += size;
      return p;
  }
  ```

- **Alternatives considered**:
  - Make `cui_arena_alloc` a wrapper around `cui_arena_alloc_aligned`: Creates a function call overhead for every allocation. Inlining the logic is trivially simple and avoids the indirection.
  - Align only at `init` and `reset`: Would only guarantee the first allocation is aligned. Subsequent allocations of varying sizes would drift out of alignment.

### 3. Should `cui_arena_alloc_aligned` change?

- **Decision**: Refactor `cui_arena_alloc_aligned` to use the same inline padding pattern (it already does). No functional change needed — it already works correctly. Keep it for callers needing alignment beyond `max_align_t` (e.g., 64-byte cache-line alignment for SIMD buffers).
- **Rationale**: The existing implementation calls `cui_arena_alloc(a, pad)` for padding, which after this change would itself add default alignment padding. This would be wasteful. Instead, `cui_arena_alloc_aligned` should compute padding and bump `a->pos` directly, without calling `cui_arena_alloc` for the padding.

### 4. What is the memory overhead?

- **Decision**: Acceptable. Worst case is `_Alignof(max_align_t) - 1` bytes per allocation (typically 7 or 15 bytes).
- **Analysis**: With a 4 MB default arena and typical `cui_node` size of ~120 bytes, a UI with 1000 nodes uses ~120 KB of arena. Adding 15 bytes of padding per node adds ~15 KB — 0.4% of the 4 MB arena. Negligible.

### 5. MSVC compatibility for `_Alignof`?

- **Decision**: Use `_Alignof` directly (C11 keyword). MSVC supports `_Alignof` since VS2015 when compiling with `/std:c11` or later.
- **Rationale**: The project already requires C11 (`-std=c11`). The MSVC equivalent `/std:c11` supports `_Alignof`. No compatibility macro needed.
- **Alternatives considered**:
  - `__alignof` (MSVC extension): Not standard C. Violates constitution principle IV.
  - `stdalign.h` with `alignof` macro: `alignof` is C23; `_Alignof` is the C11 keyword. Using `_Alignof` directly is clearest.

### 6. Should `malloc` alignment guarantee be documented?

- **Decision**: Yes. Add a brief doc comment to `cui_arena_alloc` stating the alignment guarantee.
- **Rationale**: Callers need to know what alignment they can rely on. `malloc` documents its guarantee; the arena should too.
