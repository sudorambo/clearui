# Data Model: API Polish (0.10.0)

## Entities

### Public API surface

- **Headers**: `include/clearui.h`, `include/clearui_platform.h`, `include/clearui_rdi.h`.
- **Conventions** (from review): `cui_ctx *ctx` first; optional config/opts as `const X *opts` or zero-init struct; return values consistent (int for success/click, pointer for getters).
- **Count**: Track total public functions; stay under ~120 (constitution).

### cui_layout padding semantics

- **Fields**: `padding` (shorthand), `padding_x`, `padding_y`.
- **Rule**: If `padding_x > 0` or `padding_y > 0`, use them per-axis; else if `padding > 0`, use for both. Zero-init = no padding.
- **Documentation**: Header comment and README; no struct change.

### Version

- **Compile-time**: `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH` in `clearui.h`.
- **Runtime**: `cui_version_string(void)` returns `"MAJOR.MINOR.PATCH"` (e.g. `"0.10.0"`). Implemented in one .c file; no new public state.

### Optional widgets (consider)

- **cui_image**: Widget that displays a texture/image by handle or id; bounds from opts or w/h. RDI/platform may need texture upload; scope TBD.
- **cui_tooltip** / **cui_popup**: Container or helper for transient content; behavior (hover vs click, positioning) TBD. May be deferred to 1.x.

### Documentation artifacts

- **API reference**: Generated (e.g. Doxygen) or hand-written list of public APIs; location `docs/api/` or equivalent.
- **Migration guide**: `docs/MIGRATION.md` or README section; versions 0.2.0–0.10.0 with breaking/notable changes and upgrade notes.

### Freeze

- **Policy**: After 0.10.0, no breaking changes to the public API until 1.0. Additive changes only. Document in README/CONTRIBUTING.

## State transitions

- Version macros and `cui_version_string()` output stay in sync on release.
- Layout padding behavior is documented; no state machine.

## Validation rules

- New public symbols must not push function count over ~120 without justification.
- Any new widget must follow existing patterns (ctx first, retained state via diff, a11y if interactive).
