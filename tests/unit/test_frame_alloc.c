#include "../../src/core/frame_alloc.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	cui_frame_allocator f;
	cui_frame_allocator_init(&f, 4096);

	void *p1 = cui_frame_allocator_alloc(&f, 100);
	void *p2 = cui_frame_allocator_alloc(&f, 200);
	assert(p1 != NULL && p2 != NULL);
	assert((char *)p2 - (char *)p1 == 100);

	cui_frame_allocator_reset(&f);
	void *p3 = cui_frame_allocator_alloc(&f, 50);
	assert(p3 == p1);

	cui_frame_allocator_alloc(&f, 4096);
	void *p4 = cui_frame_allocator_alloc(&f, 1);
	assert(p4 != NULL);

	void *z = cui_frame_allocator_alloc(&f, 0);
	assert(z == NULL);

	cui_frame_allocator_free(&f);
	assert(f.base == NULL && f.cap == 0 && f.pos == 0);

	printf("test_frame_alloc: PASS\n");
	return 0;
}
