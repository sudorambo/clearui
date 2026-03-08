/**
 * Linear (bump) arena allocator. Reset per frame to reclaim all UI node memory
 * without per-node free calls. Grows via realloc when capacity is exceeded,
 * which may relocate the buffer -- callers must not hold stale pointers across
 * allocations that could trigger growth.
 */
#include "arena.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef CUI_ARENA_DEFAULT_CAP
#define CUI_ARENA_DEFAULT_CAP (4 * 1024 * 1024)
#endif

/* Match malloc's alignment guarantee so arena-allocated structs are safe for any type. */
#define CUI_ARENA_DEFAULT_ALIGN _Alignof(max_align_t)

void cui_arena_init(cui_arena *a, size_t initial_cap) {
	if (initial_cap == 0) initial_cap = CUI_ARENA_DEFAULT_CAP;
	a->base = (char *)malloc(initial_cap);
	a->pos  = 0;
	a->cap  = a->base ? initial_cap : 0;
}

void cui_arena_reset(cui_arena *a) {
	a->pos = 0;
}

void cui_arena_free(cui_arena *a) {
	free(a->base);
	a->base = NULL;
	a->pos = a->cap = 0;
}

/* Double capacity until the arena can satisfy `need` more bytes. Bails on overflow.
 * Use malloc+memcpy+free instead of realloc so the old block is always explicitly
 * freed (avoids Valgrind "definitely lost" on some platforms where realloc is not
 * tracked as freeing the old block). */
static void grow(cui_arena *a, size_t need) {
	if (a->cap > SIZE_MAX / 2) return;
	size_t new_cap = a->cap ? a->cap * 2 : 256;
	while (new_cap < a->pos + need) {
		if (new_cap > SIZE_MAX / 2) return;
		new_cap *= 2;
	}
	char *n = (char *)malloc(new_cap);
	if (n) {
		if (a->base && a->pos > 0)
			memcpy(n, a->base, a->pos);
		free(a->base);
		a->base = n;
		a->cap  = new_cap;
	}
}

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

/**
 * Recomputes padding after grow() because realloc may change the base address,
 * invalidating the pre-grow alignment offset.
 */
void *cui_arena_alloc_aligned(cui_arena *a, size_t size, size_t align) {
	if (size == 0) return NULL;
	if (align == 0 || (align & (align - 1)) != 0) return NULL;
	uintptr_t cur = (uintptr_t)(a->base + a->pos);
	size_t pad = (align - (cur % align)) % align;
	size_t total = pad + size;
	if (a->pos + total > a->cap) grow(a, total);
	if (a->pos + total > a->cap) return NULL;
	cur = (uintptr_t)(a->base + a->pos);
	pad = (align - (cur % align)) % align;
	a->pos += pad;
	void *p = a->base + a->pos;
	a->pos += size;
	return p;
}
