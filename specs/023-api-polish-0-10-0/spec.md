# Feature Specification: API Polish (0.10.0)

**Version**: 0.10.0  
**Branch**: `023-api-polish-0-10-0`

## Goal

Final API review before locking the 1.0 contract: consistent signatures, clear layout padding semantics, optional new widgets (image, tooltip/popup), runtime version string, API reference docs, migration guide, and API freeze.

## User Stories

1. **US1** As a developer, I want every public function to follow consistent naming and parameter order (ctx first, opts structs, etc.) so the API is predictable and ergonomic.
2. **US2** As a developer, I want the `cui_layout.padding` vs `padding_x`/`padding_y` behavior documented or simplified (deprecate or clarify) so I know which to use.
3. **US3** As a developer, I want an optional `cui_image` widget (texture + bounds) and/or `cui_tooltip`/`cui_popup` container so common UI patterns are supported; scope to "consider" — include only if fits API cap and timeline.
4. **US4** As a developer, I want `cui_version_string()` returning e.g. `"0.10.0"` at runtime so I can log or display the library version.
5. **US5** As a maintainer, I want API reference documentation (Doxygen or custom) and a migration guide for changes since 0.1.0 so adopters can upgrade and discover APIs.
6. **US6** As a maintainer, I want the public API frozen after this milestone — no breaking changes after 0.10.0 so 1.0 is a stable contract.

## Acceptance Criteria

- All public functions in `include/clearui.h` (and related headers) reviewed for consistency (ctx first, optional opts as pointer or zero-init struct).
- `cui_layout`: either deprecate `padding` in favor of `padding_x`/`padding_y`, or document precedence clearly in header and README.
- Optional: `cui_image` and/or `cui_tooltip`/`cui_popup` — **reserved for 1.x**; not implemented in 0.10.0 to keep API surface and scope manageable.
- `cui_version_string()` added; returns string matching `CUI_VERSION_MAJOR.MINOR.PATCH` (e.g. `"0.10.0"`).
- API reference generated (Doxygen or minimal custom) and published or checked in (e.g. `docs/api/` or CI artifact).
- Migration guide (e.g. `docs/MIGRATION.md` or README section) covering breaking or notable changes since 0.1.0.
- Freeze documented: no breaking public API changes after 0.10.0; 1.0 is additive/stable.

## Out of Scope (0.10.0)

- Changing internal implementation in breaking ways that are not visible in the public API.
- Adding every possible widget; only image/tooltip/popup are in "consider" scope.

## API audit (0.10.0)

- **clearui.h**: All functions take `cui_ctx *ctx` first; opts are `const X *`. Returns: int for button/checkbox/text_input/icon_button; void for mutators; pointer for getters (cui_state, cui_frame_alloc, cui_frame_printf, cui_ctx_hovered_id). Consistent.
- **clearui_platform.h**: `cui_platform` is a struct of function pointers; each takes `cui_platform_ctx *` (or ctx + void*cui_ctx for poll_events). `cui_platform_stub_get(void)`. No changes needed.
- **clearui_rdi.h**: `cui_rdi` struct of function pointers; each takes `cui_rdi_context *`. Getters/setters follow same pattern. No changes needed.

## Requirements

- **R1** API surface remains within ~120 functions (constitution); new symbols must not push over without justification.
- **R2** C11 only; no new mandatory dependencies for version string or docs (Doxygen is optional tooling).
- **R3** Zero-to-window and clarity preserved; no hidden behavior.
