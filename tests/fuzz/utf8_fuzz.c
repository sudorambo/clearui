/**
 * Fuzz target for UTF-8 decoder (cui_utf8_next_len).
 * Build with: make fuzz-utf8 (uses -fsanitize=fuzzer). Run: ./fuzz_utf8 [corpus_dir].
 */
#include "core/utf8.h"
#include <stddef.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	if (!data) return 0;
	const unsigned char *p = (const unsigned char *)data;
	size_t remaining = size;
	while (remaining > 0) {
		int cp;
		int adv = cui_utf8_next_len(p, (int)(remaining > 4 ? 4 : remaining), &cp);
		if (adv <= 0) break;
		p += (size_t)adv;
		remaining -= (size_t)adv;
	}
	return 0;
}
