# Quickstart: Security and Robustness Hardening

**Branch**: `014-security-docs-overflow` | **Date**: 2026-03-07

This feature adds **documentation** and **defensive code** (overflow checks, test hygiene). There are no breaking API changes. Existing correct usage remains valid.

## For API users

1. **`cui_frame_printf(ctx, fmt, ...)`**  
   Use only application-controlled format strings (e.g. literals like `"%s"` or `"Value: %d"`). Do not pass user input as `fmt`. If the implementation adds a length cap, very long results will return `""`.

2. **`cui_state(ctx, key, size)` / vault APIs**  
   Pass a valid C string for `key` (NUL-terminated). Do not pass a pointer into a buffer that might not be terminated.

3. **`cui_font_measure(..., utf8, ...)`**  
   Pass a valid C string for `utf8` (NUL-terminated, valid UTF-8). Do not pass truncated or non–NUL-terminated buffers.

## For implementers

- **Arena and frame allocator**: Before doubling capacity, check `new_cap <= SIZE_MAX / 2`; on failure, do not multiply and treat allocation as failed (return NULL).
- **Tests**: In `test_vault.c`, use `snprintf(buf, sizeof(buf), "%s", "hello")` (or equivalent) instead of `strcpy` for the relevant buffer test.

## Build and test

```bash
make all
make unit-tests
make integration-tests
```

No new targets. After implementing the overflow checks and test change, run the same test suite; all existing tests should pass.

## Documentation locations

- **frame_printf**: Comment in `include/clearui.h` or `src/core/context.c` next to `cui_frame_printf` declaration/definition.
- **Vault key**: Comment in `include/clearui.h` or vault header/source for `cui_state` / `cui_vault_get`.
- **UTF-8**: Comment in `src/font/atlas.c` for `cui_font_measure` and/or `utf8_next`.
- **Contracts**: See [contracts/api-contracts-security.md](contracts/api-contracts-security.md) for the full contract text.
