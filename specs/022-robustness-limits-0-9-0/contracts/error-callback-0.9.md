# Contract: Error callback (0.9.0)

Optional application-provided callback for limit overflows and misuse.

## Purpose

When a fixed limit is exceeded (parent stack, focusables, a11y, widget ID truncation) or when CUI_DEBUG detects misuse (e.g. unbalanced push/pop), the library may call an optional error callback instead of or in addition to silent truncation / assert. This allows applications to log, break in a debugger, or adjust behavior without changing default release behavior.

## Signature (conceptual)

```c
typedef void (*cui_error_fn)(void *userdata, int error_code, const char *limit_name_or_null);
```

- **userdata**: Opaque pointer set when registering the callback.
- **error_code**: One of a small set of constants (e.g. CUI_ERR_PARENT_STACK, CUI_ERR_FOCUSABLE_FULL, CUI_ERR_A11Y_FULL, CUI_ERR_ID_TRUNCATED, CUI_ERR_UNBALANCED).
- **limit_name_or_null**: Optional string describing the limit or NULL.

## Registration

- Via `cui_config` at create time: set `config.error_callback` and `config.error_userdata` before calling `cui_create(&config)`. NULL callback = no call (current silent behavior).

## Guarantees

- Callback is invoked at most once per limit hit per frame (or per event) to avoid re-entrancy loops. The library must not call into application code that re-enters the same context in a way that could trigger the same limit again without guard.
- In release builds without a callback, behavior remains silent truncation (backward compatible).
- In CUI_DEBUG builds, assert may fire in addition to or instead of callback for parent stack overflow (implementation choice).
