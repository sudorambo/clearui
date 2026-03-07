# Quickstart: Review Fixes

**Branch**: `012-review-fixes` | **Date**: 2026-03-07

This feature is a bug-fix and improvement batch. There is no new user-facing workflow. All existing code continues to work with minimal migration (see [contracts/public-api-changes.md](contracts/public-api-changes.md)).

## Build & Test

```bash
# Build all
make all

# Run all tests (including new ones)
make unit-tests
make integration-tests

# Build static library (new)
make lib
# produces libclearui.a
```

## Migration Checklist

1. If you call `cui_set_platform()` or `cui_set_rdi()`, ensure you include `clearui_platform.h` and/or `clearui_rdi.h` (you likely already do).

2. If you use `cui_text_input_opts`, change `.placeholder = 0` to `.placeholder = NULL` or `.placeholder = "Enter text..."`.

3. If you read `cui_a11y_entry.state`, it is now a `char[32]` instead of `const char *`. Reading it as a string still works; `sizeof(cui_a11y_entry)` has increased by ~24 bytes.

4. `cui_draw_circle` now produces a visually correct circle. No code change needed.

5. `cui_arena_alloc_aligned` now returns NULL for invalid alignment values (0, non-power-of-2). If your code passed valid alignments, no change needed.
