# Quickstart: Remove Duplicate CUI_DRAW_BUF_MAX

**Branch**: `004-remove-duplicate-draw-buf-max` | **Date**: 2026-03-07

## Implementation

Delete line 10 from `src/core/render.c`:

```c
#define CUI_DRAW_BUF_MAX 1024   /* DELETE THIS LINE */
```

## Verification

```bash
# Confirm no local definition remains
grep -n 'CUI_DRAW_BUF_MAX' src/core/render.c
# Expected: no output

# Full build + tests
make clean && make all && make unit-tests && make integration-tests
# Expected: zero warnings, all tests PASS
```
