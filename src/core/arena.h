#ifndef CLEARUI_ARENA_H
#define CLEARUI_ARENA_H

#include <stddef.h>

typedef struct cui_arena {
	char  *base;
	size_t pos;
	size_t cap;
} cui_arena;

void cui_arena_init(cui_arena *a, size_t initial_cap);
void cui_arena_reset(cui_arena *a);
void cui_arena_free(cui_arena *a);

void *cui_arena_alloc(cui_arena *a, size_t size);
void *cui_arena_alloc_aligned(cui_arena *a, size_t size, size_t align);

#endif
