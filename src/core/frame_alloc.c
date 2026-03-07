#include "frame_alloc.h"
#include <stdlib.h>

#define CUI_FRAME_DEFAULT_CAP 65536

void cui_frame_allocator_init(cui_frame_allocator *f, size_t initial_cap) {
	if (initial_cap == 0) initial_cap = CUI_FRAME_DEFAULT_CAP;
	f->base = (char *)malloc(initial_cap);
	f->pos  = 0;
	f->cap  = f->base ? initial_cap : 0;
}

void cui_frame_allocator_reset(cui_frame_allocator *f) {
	f->pos = 0;
}

void cui_frame_allocator_free(cui_frame_allocator *f) {
	free(f->base);
	f->base = NULL;
	f->pos = f->cap = 0;
}

void *cui_frame_allocator_alloc(cui_frame_allocator *f, size_t size) {
	if (size == 0) return NULL;
	if (f->pos + size > f->cap) {
		size_t new_cap = f->cap ? f->cap * 2 : CUI_FRAME_DEFAULT_CAP;
		while (new_cap < f->pos + size) new_cap *= 2;
		char *n = (char *)realloc(f->base, new_cap);
		if (!n) return NULL;
		f->base = n;
		f->cap  = new_cap;
	}
	void *p = f->base + f->pos;
	f->pos += size;
	return p;
}
