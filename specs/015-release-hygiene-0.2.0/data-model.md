# Data Model: Release Hygiene (0.2.0)

No persistent runtime data model or new structures are introduced. This milestone only adds documentation and CI artifacts. The following describe the documented “contract” entities and file formats.

## Version macros (existing)

| Entity   | Location       | Description |
|----------|----------------|-------------|
| Version  | `include/clearui.h` | `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH` — numeric version for API consumers. |

No schema change; 0.2.0 will set MINOR to 2 and PATCH to 0 when released.

## CHANGELOG

| Entity    | Location     | Description |
|-----------|--------------|-------------|
| CHANGELOG | `CHANGELOG.md` | Single file. Sections per [Keep a Changelog](https://keepachangelog.com/): Unreleased, then `[X.Y.Z] - YYYY-MM-DD` with Added, Changed, Fixed, Deprecated, Removed, Security. |

Validation: human review; optional CI check that the file exists and has a version block for the tag.

## Documented limits (contract only)

These are not new data structures; they are constants already in the codebase, now documented in the public header and README. Behavior when exceeded: silent truncation (no crash, no error return).

| Constant                 | Value  | Meaning |
|--------------------------|--------|--------|
| `CUI_PARENT_STACK_MAX`   | 16     | Max container nesting depth. |
| `CUI_FOCUSABLE_MAX`      | 64     | Max focusable widgets per frame. |
| `CUI_A11Y_MAX`           | 128    | Max accessibility entries. |
| `CUI_LAST_CLICKED_ID_MAX`| 64     | Max widget ID length (bytes, including NUL). |
| `CUI_FRAME_PRINTF_MAX`   | 65536  | Max formatted string output (bytes). |

Defined in `src/core/context.c` and `src/core/a11y.h`; for 0.2.0 they are exposed in the public header (or README) as documented limits only; implementation stays in core.

## State transitions

None. Release hygiene does not add new state machines or lifecycle changes.
