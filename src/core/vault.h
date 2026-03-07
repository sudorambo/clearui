#ifndef CLEARUI_VAULT_H
#define CLEARUI_VAULT_H

/**
 * Vault: opaque persistent key-value store for cui_state.
 * Heap-allocated; survives across frames for the context's lifetime.
 */

#include <stddef.h>

typedef struct cui_vault cui_vault;

cui_vault *cui_vault_create(size_t initial_cap);
void       cui_vault_destroy(cui_vault *v);

void *cui_vault_get(cui_vault *v, const char *key, size_t size);

#endif
