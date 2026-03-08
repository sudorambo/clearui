/**
 * Shared UTF-8 decoder: overlong and surrogate rejection, no overread.
 * Used by font atlas and software RDI. See utf8.h for contract.
 */
#include "utf8.h"

#define SURROGATE_LO 0xD800
#define SURROGATE_HI 0xDFFF

static int is_cont(unsigned char c) { return c >= 0x80 && c <= 0xBF; }

int cui_utf8_next(const unsigned char *s, int *out_cp) {
	if (!s || !out_cp) return 0;
	unsigned char c = s[0];
	if (c == 0) { *out_cp = 0; return 0; }
	if (c < 0x80) {
		*out_cp = (int)c;
		return 1;
	}
	if (c < 0xC2) return 0; /* 0x80..0xBF continuation only; 0xC0,0xC1 overlong for NUL..0x7F */
	if (c < 0xE0) {
		if (!s[1] || !is_cont(s[1])) return 0;
		int cp = (int)(c & 0x1F) << 6 | (int)(s[1] & 0x3F);
		if (cp < 0x80) return 0; /* overlong */
		if (cp >= SURROGATE_LO && cp <= SURROGATE_HI) return 0;
		*out_cp = cp;
		return 2;
	}
	if (c < 0xF0) {
		if (!s[1] || !is_cont(s[1]) || !s[2] || !is_cont(s[2])) return 0;
		int cp = (int)(c & 0x0F) << 12 | (int)(s[1] & 0x3F) << 6 | (int)(s[2] & 0x3F);
		if (cp < 0x800) return 0; /* overlong */
		if (cp >= SURROGATE_LO && cp <= SURROGATE_HI) return 0;
		*out_cp = cp;
		return 3;
	}
	if (c < 0xF8) {
		if (!s[1] || !is_cont(s[1]) || !s[2] || !is_cont(s[2]) || !s[3] || !is_cont(s[3])) return 0;
		int cp = (int)(c & 0x07) << 18 | (int)(s[1] & 0x3F) << 12 | (int)(s[2] & 0x3F) << 6 | (int)(s[3] & 0x3F);
		if (cp < 0x10000) return 0; /* overlong */
		if (cp > 0x10FFFF) return 0; /* beyond Unicode */
		if (cp >= SURROGATE_LO && cp <= SURROGATE_HI) return 0;
		*out_cp = cp;
		return 4;
	}
	return 0; /* invalid lead */
}

int cui_utf8_next_len(const unsigned char *s, int max_len, int *out_cp) {
	if (!s || !out_cp || max_len <= 0) return 0;
	unsigned char c = s[0];
	if (c == 0) { *out_cp = 0; return 0; }
	if (c < 0x80) {
		*out_cp = (int)c;
		return 1;
	}
	if (c < 0xC2) return 0;
	if (c < 0xE0) {
		if (max_len < 2 || !is_cont(s[1])) return 0;
		int cp = (int)(c & 0x1F) << 6 | (int)(s[1] & 0x3F);
		if (cp < 0x80 || (cp >= SURROGATE_LO && cp <= SURROGATE_HI)) return 0;
		*out_cp = cp;
		return 2;
	}
	if (c < 0xF0) {
		if (max_len < 3 || !is_cont(s[1]) || !is_cont(s[2])) return 0;
		int cp = (int)(c & 0x0F) << 12 | (int)(s[1] & 0x3F) << 6 | (int)(s[2] & 0x3F);
		if (cp < 0x800 || (cp >= SURROGATE_LO && cp <= SURROGATE_HI)) return 0;
		*out_cp = cp;
		return 3;
	}
	if (c < 0xF8) {
		if (max_len < 4 || !is_cont(s[1]) || !is_cont(s[2]) || !is_cont(s[3])) return 0;
		int cp = (int)(c & 0x07) << 18 | (int)(s[1] & 0x3F) << 12 | (int)(s[2] & 0x3F) << 6 | (int)(s[3] & 0x3F);
		if (cp < 0x10000 || cp > 0x10FFFF || (cp >= SURROGATE_LO && cp <= SURROGATE_HI)) return 0;
		*out_cp = cp;
		return 4;
	}
	return 0;
}
