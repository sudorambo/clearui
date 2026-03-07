/**
 * Vault: persistent key-value store for application state (cui_state).
 * Open-addressed hash table with linear probing and FNV-1a hashing.
 * Grows at 50% load factor. Values are heap-allocated, zero-initialized on
 * first access, and stable (same pointer returned on subsequent lookups).
 */
#include "vault.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char *dup_str(const char *s) {
	size_t n = strlen(s) + 1;
	char *p = (char *)malloc(n);
	if (p) memcpy(p, s, n);
	return p;
}

#define CUI_VAULT_INITIAL_CAP 32

/* FNV-1a 64-bit: fast, well-distributed hash for short string keys. */
static unsigned long long fnv1a(const char *s) {
	unsigned long long h = 14695981039346656037ULL;
	while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211ULL; }
	return h;
}

struct slot {
	unsigned long long hash;
	char              *key;
	void              *value;
	size_t             size;
};

struct cui_vault {
	struct slot *slots;
	size_t       cap;
	size_t       count;
};

cui_vault *cui_vault_create(size_t initial_cap) {
	if (initial_cap == 0) initial_cap = CUI_VAULT_INITIAL_CAP;
	cui_vault *v = (cui_vault *)malloc(sizeof(cui_vault));
	if (!v) return NULL;
	v->slots = (struct slot *)calloc(initial_cap, sizeof(struct slot));
	v->cap   = v->slots ? initial_cap : 0;
	v->count = 0;
	if (!v->slots) { free(v); return NULL; }
	return v;
}

void cui_vault_destroy(cui_vault *v) {
	if (!v) return;
	for (size_t i = 0; i < v->cap; i++) {
		free(v->slots[i].key);
		free(v->slots[i].value);
	}
	free(v->slots);
	free(v);
}

static struct slot *find(cui_vault *v, const char *key, unsigned long long hash) {
	size_t i = hash % v->cap;
	for (;;) {
		if (v->slots[i].value == NULL && v->slots[i].key == NULL) return &v->slots[i];
		if (v->slots[i].hash == hash && v->slots[i].key && strcmp(v->slots[i].key, key) == 0)
			return &v->slots[i];
		i = (i + 1) % v->cap;
	}
}

static int expand(cui_vault *v) {
	/* Avoid integer overflow when doubling capacity */
	if (v->cap > SIZE_MAX / 2) return -1;
	size_t new_cap = v->cap * 2;
	struct slot *n = (struct slot *)calloc(new_cap, sizeof(struct slot));
	if (!n) return -1;
	for (size_t i = 0; i < v->cap; i++) {
		if (v->slots[i].key) {
			size_t j = v->slots[i].hash % new_cap;
			while (n[j].key) j = (j + 1) % new_cap;
			n[j] = v->slots[i];
		}
	}
	free(v->slots);
	v->slots = n;
	v->cap   = new_cap;
	return 0;
}

/**
 * Get-or-create: returns existing value for key, or allocates a new
 * zero-initialized block of `size` bytes. Must re-probe after expand()
 * because slot pointers are invalidated by the reallocation.
 */
void *cui_vault_get(cui_vault *v, const char *key, size_t size) {
	if (!v || !key) return NULL;
	unsigned long long h = fnv1a(key);
	struct slot *s = find(v, key, h);
	if (s->key) return s->value;
	if (v->count >= v->cap / 2 && expand(v) != 0) return NULL;
	s = find(v, key, h);
	s->hash  = h;
	s->key   = dup_str(key);
	s->size  = size;
	s->value = calloc(1, size);
	if (!s->key || !s->value) {
		free(s->key);
		free(s->value);
		s->key = NULL;
		s->value = NULL;
		return NULL;
	}
	v->count++;
	return s->value;
}
