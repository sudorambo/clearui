# Contract: UTF-8 robustness (0.9.0)

Behavior of the internal UTF-8 decoder (`utf8_next` or shared helper) used by font measurement and software RDI text.

## Input contract

- **NUL-terminated**: Callers (e.g. `cui_font_measure`, soft RDI text) must pass a valid C string (NUL-terminated) unless a length-bound API is introduced.
- **Length-bound variant** (optional for 0.9.0): If a function accepts `(const char *buf, size_t len)`, the decoder must not read past `buf + len` and must not assume NUL at `buf[len]`.

## Output and validation

- **Well-formed UTF-8**: Accept only well-formed sequences. Reject overlong encodings (e.g. C0 80 for U+0000).
- **Surrogates**: Reject or replace UTF-16 surrogates (U+D800–U+DFFF) if they appear in UTF-8 input.
- **Truncated sequence**: If a multi-byte lead byte is followed by insufficient bytes (or end of buffer without NUL), do not advance past the buffer; return 0 advance and optionally a replacement codepoint (e.g. U+FFFD) or leave output undefined for that call. No buffer overread.
- **Return**: Advance in bytes (0 on error or end); output codepoint in *out_cp.

## Placement

- Shared implementation in one place (e.g. `src/core/utf8.h` or in `src/font/atlas.c`) and used by both font and soft RDI, or duplicated with identical rules. Document location and contract in source comments.
