# Quickstart: Robustness & Limits (0.9.0)

## Using the error callback

1. When creating the context, set an optional error callback in `cui_config` (or call `cui_set_error_callback` after create if the API provides it):
   - `error_callback`: function pointer `void (*)(void *userdata, int error_code, const char *limit_name_or_null)`.
   - `error_userdata`: passed as first argument when the callback is invoked.
2. When a limit is hit (parent stack full, focusable list full, a11y full, widget ID truncated), the library calls your callback with the appropriate error code. You can log, break in a debugger, or ignore.
3. If you do not set a callback, behavior is unchanged (silent truncation).

## CUI_DEBUG builds

- Compile with `-DCUI_DEBUG` to enable assertions for unbalanced push/pop and parent stack overflow. Useful for development; leave undefined in release.

## Fuzz and stress tests

- **Fuzz**: `make fuzz-utf8`, `make fuzz-vault`, `make fuzz-frame` build the UTF-8, vault, and frame allocator fuzz targets (libFuzzer). Run `./fuzz_utf8 [corpus_dir]` or `./fuzz_utf8 -runs=10000` (and similarly for `fuzz_vault`, `fuzz_frame`).
- **Stress**: `make stress` builds and runs a stress test (1200 widgets × 10 frames); no crash.

## Memory leak CI

- `make leak-check` runs all unit tests under Valgrind (`--leak-check=full --error-exitcode=1`). Requires Valgrind on Linux. CI job `leak-check` runs this on every push.
