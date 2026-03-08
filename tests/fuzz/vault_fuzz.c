/**
 * Fuzz target for vault (cui_vault_create, cui_vault_get, cui_vault_destroy).
 * Build with: make fuzz-vault. Run: ./fuzz_vault [corpus_dir].
 */
#include "core/vault.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define KEY_BUF_MAX 64
#define VALUE_SIZE_MAX 4096

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	if (!data || size < 2) return 0;

	cui_vault *v = cui_vault_create(8);
	if (!v) return 0;

	const uint8_t *p = data;
	const uint8_t *end = data + size;

	while (p + 2 <= end) {
		unsigned klen = p[0] % KEY_BUF_MAX;
		if (klen == 0) klen = 1;
		p++;
		size_t val_size = (size_t)(p[0]) * 16 + 1; /* 1..4097, clamp below */
		if (val_size > VALUE_SIZE_MAX) val_size = VALUE_SIZE_MAX;
		p++;

		char key[KEY_BUF_MAX];
		size_t copy = (size_t)klen;
		if (p + copy > end) copy = (size_t)(end - p);
		memcpy(key, p, copy);
		key[copy] = '\0';
		p += copy;

		(void)cui_vault_get(v, key, val_size);
	}

	cui_vault_destroy(v);
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
