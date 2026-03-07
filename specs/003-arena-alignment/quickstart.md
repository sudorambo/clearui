# Quickstart: Add Default Alignment to Arena Allocator

**Branch**: `003-arena-alignment` | **Date**: 2026-03-07

## Implementation Steps

### Step 1: Modify `cui_arena_alloc` in `src/core/arena.c`

Add alignment padding before the bump. Use `_Alignof(max_align_t)` as the default alignment:

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

### Step 2: Refactor `cui_arena_alloc_aligned` in `src/core/arena.c`

Avoid calling `cui_arena_alloc` for padding (which would add double-alignment overhead):

```c
void *cui_arena_alloc_aligned(cui_arena *a, size_t size, size_t align) {
    if (size == 0) return NULL;
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

### Step 3: Add alignment test to `tests/unit/test_arena.c`

```c
#include <stdint.h>
#include <stdalign.h>

/* Verify alignment guarantee */
void *p = cui_arena_alloc(&a, 1);    /* small alloc */
assert(((uintptr_t)p % _Alignof(max_align_t)) == 0);

void *q = cui_arena_alloc(&a, 37);   /* odd size */
assert(((uintptr_t)q % _Alignof(max_align_t)) == 0);

void *r = cui_arena_alloc(&a, sizeof(cui_node));  /* struct alloc */
assert(((uintptr_t)r % _Alignof(max_align_t)) == 0);
```

## Verification

### Build and test

```bash
make clean && make all && make unit-tests && make integration-tests
```

Expected: zero warnings, all tests PASS.

### Manual alignment check

```bash
# Compile and run test_arena — new assertions verify alignment
make test_arena && ./test_arena
```
