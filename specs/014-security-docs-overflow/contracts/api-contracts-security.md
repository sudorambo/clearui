# API Contracts: Security and Robustness (014)

**Branch**: `014-security-docs-overflow` | **Date**: 2026-03-07

This document records the **contracts** that callers must satisfy when using the following APIs. No function signatures or struct layouts change in this feature; only documentation and (where implemented) defensive checks are added.

---

## 1. cui_frame_printf

```c
const char *cui_frame_printf(cui_ctx *ctx, const char *fmt, ...);
```

**Contract**:

- `fmt` MUST be **application-controlled**: a format string chosen by the application (e.g. string literal or internal constant). It MUST NOT be user-controlled input.
- **Rationale**: If `fmt` is user-controlled, (1) format-string attacks are possible if the result is used in another format context, and (2) a format that produces a very large length can cause unbounded allocation (DoS).
- **Optional implementation**: The implementation MAY enforce a maximum formatted length (e.g. 65536 bytes). If the required length exceeds the cap (or if `vsnprintf` returns negative), the function MUST return `""` and MUST NOT allocate. This is an implementation detail; the contract above is the primary requirement.

**Documentation placement**: API comment in header or source; optionally a “Security / robustness” section in project docs.

---

## 2. Vault key parameter (cui_vault_get, cui_state)

```c
void *cui_vault_get(cui_vault *v, const char *key, size_t size);
void *cui_state(cui_ctx *ctx, const char *key, size_t size);
```

**Contract**:

- `key` MUST be a **valid C string**: NUL-terminated, and the buffer it points to must be readable up to and including the terminating NUL.
- **Rationale**: Internal code uses `strlen(key)` and copies with `strlen(key)+1`; a non–NUL-terminated buffer causes undefined behavior.
- **Future**: A length-bounded API (e.g. `cui_vault_get_n(..., key, key_len, size)`) may be added for untrusted keys; not part of this feature.

**Documentation placement**: Comment on the `key` parameter in the header or vault API section.

---

## 3. UTF-8 string parameter (cui_font_measure and internal utf8_next)

```c
void cui_font_measure(int font_id, int font_size_px, const char *utf8, float *out_width, float *out_height);
// Internal: static int utf8_next(const unsigned char *s, int *out_cp);
```

**Contract**:

- `utf8` (and any pointer passed to `utf8_next`) MUST refer to a **valid C string**: NUL-terminated and containing valid UTF-8. The buffer MUST be at least as long as the encoded representation of the text (i.e. no truncated multi-byte sequence at the end).
- **Rationale**: `utf8_next` reads up to 4 bytes (`s[0]`–`s[3]`) with only truthiness checks; a short or non–NUL-terminated buffer can cause buffer overread.
- **Future**: A length-taking API (e.g. `cui_font_measure_n(..., utf8, len)`) or internal bounds-checked variant may be added for untrusted input; not part of this feature.

**Documentation placement**: Comment on the `utf8` parameter and/or at the top of the font/atlas module or `utf8_next`.

---

## 4. Arena and frame allocator (internal)

These are not public API in the same sense; the **implementation** is constrained:

- Before any operation that would do `new_cap *= 2` (or equivalent), the implementation MUST check that `new_cap <= SIZE_MAX / 2`. If the check fails, the implementation MUST NOT perform the multiplication and MUST treat the allocation as failed (e.g. return NULL).
- No new public types or functions; the contract is for implementers and maintainers.

---

## Summary table

| API / component      | Parameter / behavior | Contract                                      |
|----------------------|----------------------|-----------------------------------------------|
| `cui_frame_printf`   | `fmt`                | Application-controlled only; optional length cap. |
| `cui_vault_get` / `cui_state` | `key`         | Valid C string (NUL-terminated).               |
| `cui_font_measure`   | `utf8`               | Valid C string, valid UTF-8.                   |
| Arena / frame grow   | Internal             | Overflow check before doubling capacity.      |
