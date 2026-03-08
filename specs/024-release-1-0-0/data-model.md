# Data Model: 1.0.0 Release

## Entities

### Version

- **CUI_VERSION_MAJOR**: 1 (was 0)
- **CUI_VERSION_MINOR**: 0 (was 10)
- **CUI_VERSION_PATCH**: 0 (unchanged)
- **cui_version_string()**: Returns `"1.0.0"` (derived from macros).

### Release artifacts

- **Git tag**: `v1.0.0` on main branch.
- **GitHub release**: Title "ClearUI 1.0.0"; body from CHANGELOG 1.0.0 section.
- **CHANGELOG.md**: New `## [1.0.0]` entry.

### Verification matrix

| Check | Platform | Tool |
|-------|----------|------|
| Build + unit + integration | Ubuntu, macOS, Windows | `make all unit-tests integration-tests` |
| ASan | Ubuntu | `make asan` |
| UBSan | Ubuntu | `make ubsan` |
| LeakSanitizer | Ubuntu | `make leak-check-lsan` |
| Demo (headless) | Any | `make demo && ./demo` |
| Demo (window) | Linux/SDL3 | `make demo WITH_SDL3=1 && ./demo` |

## Validation rules

- Version macros must be `1, 0, 0`.
- All CI jobs pass before tagging.
- CHANGELOG must have entries for each milestone 0.2.0–0.10.0 plus 1.0.0.
- No new public API symbols (API frozen since 0.10.0).
