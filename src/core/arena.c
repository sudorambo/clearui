#include "arena.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef CUI_ARENA_DEFAULT_CAP
#define CUI_ARENA_DEFAULT_CAP (4 * 1024 * 1024)
#endif

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

static void grow(cui_arena *a, size_t need) {
	size_t new_cap = a->cap ? a->cap * 2 : 256;
	while (new_cap < a->pos + need) new_cap *= 2;
	char *n = (char *)realloc(a->base, new_cap);
	if (n) {
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

void *cui_arena_alloc_aligned(cui_arena *a, size_t size, size_t align) {
	if (size == 0) return NULL;
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
