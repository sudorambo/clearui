# Quickstart: API Polish (0.10.0)

## For implementers

1. **API review**  
   Walk every public function in `include/clearui.h` (and related headers). Ensure: `ctx` first, optional opts as `const X *`, consistent return conventions. Fix any naming or parameter-order inconsistencies; avoid breaking changes where possible (prefer additive fixes).

2. **Layout padding**  
   In `clearui.h`, expand the comment on `cui_layout` to state: "If `padding_x` or `padding_y` is greater than 0, they override `padding` for that axis; otherwise `padding` sets both. Zero-init = no padding." Update README if there is a layout or theming section.

3. **Version string**  
   Add `const char *cui_version_string(void);` to `clearui.h`. Implement in one .c file (e.g. in core) so it returns `"CUI_VERSION_MAJOR.MINOR.PATCH"`. Keep macros as single source of truth (e.g. snprintf from macros into a static buffer, or a generated literal).

4. **Optional widgets**  
   Only if within scope and API cap: add `cui_image` and/or `cui_tooltip`/`cui_popup` per research; otherwise leave as "reserved for 1.x" in the spec.

5. **API reference**  
   Add Doxygen comments to public APIs (brief + params). Add a minimal `Doxyfile` and a Makefile/CI target to generate docs into `docs/api/` (or equivalent). Alternatively, maintain a single Markdown API list.

6. **Migration guide**  
   Add `docs/MIGRATION.md` (or a Migration section in README) with one subsection per version from 0.2.0 to 0.10.0: list breaking or notable API changes and short upgrade notes.

7. **Freeze**  
   In README or CONTRIBUTING, state that the public API is frozen after 0.10.0 and that there will be no breaking changes until 1.0.

## For users

- **Version at runtime**: Call `cui_version_string()` to get a string like `"0.10.0"` for logging or about dialogs.
- **Layout padding**: Use `padding` for uniform padding, or `padding_x`/`padding_y` for per-axis; explicit `padding_x`/`padding_y` override `padding` when > 0.
- **Upgrading**: See `docs/MIGRATION.md` (or README Migration section) for changes between 0.1.0 and 0.10.0.
