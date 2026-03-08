/**
 * Shared UTF-8 decoder for ClearUI (font atlas, soft RDI).
 * Contract: input is NUL-terminated or length-bound; rejects overlong sequences
 * and surrogates (U+D800–U+DFFF); no read past buffer. Return 0 advance on error.
 */
#ifndef CLEARUI_UTF8_H
#define CLEARUI_UTF8_H

/**
 * Decode next UTF-8 codepoint at s.
 * s must be NUL-terminated (valid C string). On success: *out_cp = codepoint, return byte advance (1–4).
 * On end of string: *out_cp = 0, return 0. On error (overlong, surrogate, truncated): return 0, *out_cp undefined.
 * Rejects overlong encodings (e.g. C0 80 for U+0000) and UTF-16 surrogates (U+D800–U+DFFF).
 */
int cui_utf8_next(const unsigned char *s, int *out_cp);

/**
 * Length-bound variant: decode at s with at most max_len bytes (no NUL required).
 * Does not read past s + max_len. Return 0 advance on error or when max_len is 0.
 */
int cui_utf8_next_len(const unsigned char *s, int max_len, int *out_cp);

#endif
