/**
 * Unit test for font atlas text measurement.
 */
#include "../../src/font/atlas.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	float w1 = 0.f, h1 = 0.f, w2 = 0.f, h2 = 0.f;
	cui_font_measure(cui_font_default_id(), 16, "Hello", &w1, &h1);
	cui_font_measure(cui_font_default_id(), 16, "Hi", &w2, &h2);

	assert(w1 > w2 && "Hello should be wider than Hi");
	assert(h1 > 0.f && h2 > 0.f && "Heights should be positive");

	printf("test_font: PASS\n");
	return 0;
}
