# Contract: Public API (clearui.h)

This contract must be reflected in `include/clearui.h` and in README so that users and implementors have a single source of truth. 0.2.0 adds no new API functions; it only formalizes documentation of existing behavior.

## Color format

- **Format**: 32-bit packed color `0xAARRGGBB`.
- **Byte order**: Alpha in high byte, red next, then green, then blue in low byte.
- **Usage**: All color parameters (e.g. `cui_style.text_color`, `cui_draw_rect(..., color)`) use this format. Opaque black is `0xff000000`; opaque white is `0xffffffff`.

## Single-threaded contract

- **Rule**: All ClearUI API calls MUST be made from a single thread.
- **Implication**: No locking is performed; calling from multiple threads is undefined behavior. The application is responsible for ensuring that only one thread invokes `cui_*` functions (including `cui_begin_frame` / `cui_end_frame` and all widget/layout calls).

## Version macros

- **Names**: `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH`.
- **Semantics**: Integer macros indicating the library version at compile time. Follow semantic versioning; 0.2.0 implies MINOR=2, PATCH=0.

## Fixed limits and silent truncation

When a limit is exceeded, the library MUST NOT crash and MUST NOT return an error; it MUST truncate or cap behavior silently. Documented limits:

| Constant                 | Value  | Behavior when exceeded |
|--------------------------|--------|--------------------------|
| `CUI_PARENT_STACK_MAX`   | 16     | Max container nesting depth; deeper nesting is not supported (behavior is implementation-defined but safe). |
| `CUI_FOCUSABLE_MAX`      | 64     | Max focusable widgets per frame; additional focusables are ignored for focus list. |
| `CUI_A11Y_MAX`           | 128    | Max accessibility entries; additional entries are dropped. |
| `CUI_LAST_CLICKED_ID_MAX`| 64     | Widget ID string length in bytes (including NUL); longer IDs are truncated. |
| `CUI_FRAME_PRINTF_MAX`   | 65536  | Max output size for `cui_frame_printf` in bytes; larger output is truncated or empty. |

These limits MUST be documented in the public header (as comments or as macros re-exported for documentation) and in README. Implementation may keep the actual defines in core; the contract is that the documented values and silent-truncation behavior are guaranteed.
