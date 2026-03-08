/**
 * Fuzz target for frame allocator (init, alloc, reset, free).
 * Build with: make fuzz-frame. Run: ./fuzz_frame [corpus_dir].
 */
#include "core/frame_alloc.h"
#include <stddef.h>
#include <stdint.h>

#define ALLOC_SIZE_MAX 1024
#define ROUNDS 4

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	if (!data) return 0;

	cui_frame_allocator f;
	cui_frame_allocator_init(&f, 256);
	if (!f.base) return 0;

	for (int round = 0; round < ROUNDS && size > 0; round++) {
		const uint8_t *p = data;
		const uint8_t *end = data + size;
		while (p < end) {
			size_t alloc_size = (size_t)(*p) + 1;
			if (alloc_size > ALLOC_SIZE_MAX) alloc_size = ALLOC_SIZE_MAX;
			void *ptr = cui_frame_allocator_alloc(&f, alloc_size);
			(void)ptr;
			p++;
		}
		cui_frame_allocator_reset(&f);
	}

	cui_frame_allocator_free(&f);
	return 0;
}

#ifdef NO_LIBFUZZER
#include <stdio.h>
int main(void) {
	uint8_t buf[4096];
	size_t n = fread(buf, 1, sizeof(buf), stdin);
	LLVMFuzzerTestOneInput(buf, n);
	return 0;
}
#endif
