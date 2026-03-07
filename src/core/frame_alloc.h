#ifndef CLEARUI_FRAME_ALLOC_H
#define CLEARUI_FRAME_ALLOC_H

#include <stddef.h>

typedef struct cui_frame_allocator {
	char  *base;
	size_t pos;
	size_t cap;
} cui_frame_allocator;

void cui_frame_allocator_init(cui_frame_allocator *f, size_t initial_cap);
void cui_frame_allocator_reset(cui_frame_allocator *f);
void cui_frame_allocator_free(cui_frame_allocator *f);

void *cui_frame_allocator_alloc(cui_frame_allocator *f, size_t size);

#endif
