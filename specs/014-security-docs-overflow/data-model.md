# Data Model: Security and Robustness Hardening

**Branch**: `014-security-docs-overflow` | **Date**: 2026-03-07

## Summary

This feature does not introduce new entities or change existing struct layouts. It clarifies **contracts** for existing APIs and adds **internal** overflow checks. The data model is unchanged; only documentation and defensive logic are added.

## Contract Clarifications (Documentation)

### cui_frame_printf

- **Function**: `const char *cui_frame_printf(cui_ctx *ctx, const char *fmt, ...);`
- **Contract**: `fmt` MUST be application-controlled (e.g. a string literal or a format string chosen by the application). It MUST NOT be user-controlled input, to avoid format-string vulnerabilities and unbounded allocation (DoS).
- **Optional implementation**: If a maximum formatted length (e.g. 64K) is enforced, a result that would exceed that length MUST result in returning `""` (and no allocation), or a truncated buffer per design.

### Font / UTF-8 APIs

- **Relevant**: `cui_font_measure(int font_id, int font_size_px, const char *utf8, ...)` and internal `utf8_next(const unsigned char *s, int *out_cp)`.
- **Contract**: The `utf8` (or `s`) pointer MUST refer to a valid C string: NUL-terminated and containing valid UTF-8. If the buffer is not NUL-terminated or is shorter than the encoded character, behavior is undefined (buffer overread).
- **Future**: A length-taking API (e.g. `cui_font_measure_n(..., const char *utf8, size_t len)`) can be added for untrusted input; out of scope for this feature.

### Vault key parameters

- **Relevant**: `cui_vault_get(cui_vault *v, const char *key, size_t size)`, `cui_state(cui_ctx *ctx, const char *key, size_t size)`, and any other API that takes a vault key.
- **Contract**: `key` MUST be a valid C string (NUL-terminated). Internal use of `strlen(key)` and `dup_str(key)` assumes this; violation is undefined behavior.
- **Future**: A length-bounded variant (e.g. `cui_vault_get_n(..., key, key_len, size)`) can be considered for untrusted keys; out of scope for this feature.

## Internal Behavior Changes (No Public Data Model Change)

### Arena (cui_arena)

- **Grow logic**: Before `new_cap *= 2`, check `new_cap <= SIZE_MAX / 2`. If the check fails, do not multiply; treat as allocation failure (e.g. `grow()` does not update capacity; alloc returns NULL).
- **No new fields**: Struct unchanged.

### Frame allocator (cui_frame_allocator)

- **Grow logic**: Same as arena — check before doubling; on overflow, allocation fails (return NULL).
- **No new fields**: Struct unchanged.

## Test-Only Change

- **test_vault.c**: Replace `strcpy(dest, "hello")` (or equivalent) with `snprintf(dest, sizeof(dest), "%s", "hello")` (or equivalent bounded copy). No change to vault or context data model.
