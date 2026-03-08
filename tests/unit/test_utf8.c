/**
 * Unit test for shared UTF-8 decoder (cui_utf8_next, cui_utf8_next_len).
 * Verifies: overlong rejection, surrogate rejection, truncated input (0 advance), no overread.
 */
#include "core/utf8.h"
#include <assert.h>
#include <string.h>

static void test_ascii(void) {
	const unsigned char *s = (const unsigned char *)"A";
	int cp;
	int adv = cui_utf8_next(s, &cp);
	assert(adv == 1 && cp == 65);
	adv = cui_utf8_next(s + 1, &cp);
	assert(adv == 0 && cp == 0);
}

static void test_overlong_nul(void) {
	/* C0 80 is overlong encoding for U+0000 */
	const unsigned char s[] = { 0xC0u, 0x80u, 0 };
	int cp;
	int adv = cui_utf8_next(s, &cp);
	assert(adv == 0 && "overlong C0 80 must be rejected");
}

static void test_surrogate(void) {
	/* ED A0 80 = U+D800 (high surrogate) */
	const unsigned char s[] = { 0xEDu, 0xA0u, 0x80u, 0 };
	int cp;
	int adv = cui_utf8_next(s, &cp);
	assert(adv == 0 && "surrogate U+D800 must be rejected");
}

static void test_truncated(void) {
	/* E0 80 = truncated 3-byte (no third byte); do not read past buffer */
	const unsigned char s[] = { 0xE0u, 0x80u, 0 }; /* NUL-terminated */
	int cp;
	int adv = cui_utf8_next(s, &cp);
	assert(adv == 0 && "truncated 3-byte must return 0 advance");
}

static void test_next_len_truncated(void) {
	/* Length-bound: only 2 bytes available for 3-byte sequence */
	const unsigned char s[] = { 0xE0u, 0x80u }; /* no NUL in bound */
	int cp;
	int adv = cui_utf8_next_len(s, 2, &cp);
	assert(adv == 0 && "cui_utf8_next_len must not read past max_len");
}

static void test_valid_utf8(void) {
	const unsigned char *s = (const unsigned char *)"\xC2\xA2"; /* U+00A2 cent */
	int cp;
	int adv = cui_utf8_next(s, &cp);
	assert(adv == 2 && cp == 0xA2);
}

static void test_valid_three(void) {
	const unsigned char *s = (const unsigned char *)"\xE2\x82\xAC"; /* U+20AC euro */
	int cp;
	int adv = cui_utf8_next(s, &cp);
	assert(adv == 3 && cp == 0x20AC);
}

int main(void) {
	test_ascii();
	test_overlong_nul();
	test_surrogate();
	test_truncated();
	test_next_len_truncated();
	test_valid_utf8();
	test_valid_three();
	return 0;
}
