#include "core/arena.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

int main(void) {
	cui_arena a;
	cui_arena_init(&a, 4096);

	void *p1 = cui_arena_alloc(&a, 100);
	void *p2 = cui_arena_alloc(&a, 200);
	assert(p1 != NULL && p2 != NULL);
	assert((char *)p2 - (char *)p1 >= 100);

	cui_arena_reset(&a);
	void *p3 = cui_arena_alloc(&a, 50);
	assert(p3 == p1);

	cui_arena_alloc(&a, 4096);
	void *p4 = cui_arena_alloc(&a, 1);
	assert(p4 != NULL);

	cui_arena_free(&a);
	return 0;
}
