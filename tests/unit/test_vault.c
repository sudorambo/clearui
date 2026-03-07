#include "core/vault.h"
#include <assert.h>
#include <string.h>

int main(void) {
	cui_vault *v = cui_vault_create(0);
	assert(v != NULL);

	int *x = (int *)cui_vault_get(v, "counter", sizeof(int));
	assert(x != NULL);
	assert(*x == 0);
	*x = 42;

	int *y = (int *)cui_vault_get(v, "counter", sizeof(int));
	assert(y == x);
	assert(*y == 42);

	char *str = (char *)cui_vault_get(v, "name", 64);
	assert(str != NULL);
	strcpy(str, "hello");

	char *str2 = (char *)cui_vault_get(v, "name", 64);
	assert(str2 == str);
	assert(strcmp(str2, "hello") == 0);

	cui_vault_destroy(v);
	return 0;
}
