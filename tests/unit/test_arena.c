#include "../../src/core/arena.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ASSERT_ALIGNED(ptr) \
	assert(((uintptr_t)(ptr) % _Alignof(max_align_t)) == 0)

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

	/* T005: alignment of individual allocations */
	cui_arena_reset(&a);
	void *a1 = cui_arena_alloc(&a, 1);
	ASSERT_ALIGNED(a1);
	void *a2 = cui_arena_alloc(&a, 37);
	ASSERT_ALIGNED(a2);
	void *a3 = cui_arena_alloc(&a, sizeof(void *));
	ASSERT_ALIGNED(a3);

	/* T006: alignment holds across sequential varying-size allocations */
	cui_arena_reset(&a);
	void *s1 = cui_arena_alloc(&a, 1);
	void *s2 = cui_arena_alloc(&a, 7);
	void *s3 = cui_arena_alloc(&a, 33);
	void *s4 = cui_arena_alloc(&a, 3);
	void *s5 = cui_arena_alloc(&a, 100);
	ASSERT_ALIGNED(s1);
	ASSERT_ALIGNED(s2);
	ASSERT_ALIGNED(s3);
	ASSERT_ALIGNED(s4);
	ASSERT_ALIGNED(s5);

	/* T007: cui_arena_alloc_aligned with custom alignment (64 bytes) */
	cui_arena_reset(&a);
	void *c1 = cui_arena_alloc_aligned(&a, 128, 64);
	assert(c1 != NULL);
	assert(((uintptr_t)c1 % 64) == 0);
	void *c2 = cui_arena_alloc_aligned(&a, 10, 64);
	assert(c2 != NULL);
	assert(((uintptr_t)c2 % 64) == 0);

	cui_arena_free(&a);
	return 0;
}
