# Contract: API Freeze (0.10.0)

## Scope

Public API of ClearUI as exposed in `include/clearui.h`, `include/clearui_platform.h`, and `include/clearui_rdi.h`.

## Guarantees after 0.10.0

- **No breaking changes** until 1.0: existing function signatures, type names, and macro names must not change in incompatible ways.
- **Additive only**: new functions, optional parameters (via new overloads or opts structs), and new macros are allowed.
- **Documentation**: README or CONTRIBUTING will state that the public API is frozen and that breaking changes are reserved for a future major version.

## Out of scope

- Internal (non-public) symbols and file layout may change.
- Build system and optional dependencies (e.g. Doxygen) are not part of the API contract.
