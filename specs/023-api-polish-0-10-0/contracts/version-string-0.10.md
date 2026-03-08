# Contract: cui_version_string (0.10.0)

## Declaration

```c
const char *cui_version_string(void);
```

- Declared in `include/clearui.h`.
- No parameters; returns a pointer to a string that reflects the library version at runtime.

## Semantics

- Return value: string in the form `"MAJOR.MINOR.PATCH"` (e.g. `"0.10.0"`), consistent with `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH`.
- The returned pointer must be valid for the lifetime of the process; no allocation required (e.g. static string or literal).
- Thread-safety: not required to be thread-safe; typical use is single-threaded init or logging.

## Implementation notes

- May be implemented as a static buffer filled at first call, or as a string literal built at compile time (e.g. via macros). Must stay in sync with the three version macros on release.
