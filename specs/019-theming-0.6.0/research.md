# Research: Theming (0.6.0)

**Branch**: `019-theming-0.6.0` | **Date**: 2026-03-07

## Decisions

### Theme struct layout and ownership

**Decision**: Define a `cui_theme` struct in the public API (e.g. in `clearui.h`) with fields matching current `theme.h`: `text_color`, `button_bg`, `checkbox_bg`, `input_bg`, `corner_radius`, `font_size`, `focus_ring_color`, `focus_ring_width`. Context stores a **copy** of the theme (not a pointer). When the app calls `cui_set_theme(ctx, &theme)`, the context copies the struct; when the app calls `cui_set_theme(ctx, NULL)`, the context resets to the built-in default (copy default into the context’s theme slot). App does not need to keep the theme struct alive after the call.

**Rationale**: Copy semantics avoid lifetime bugs (app freeing or reusing the theme). No extra allocation; the context already has a fixed-size struct. Matches “explicit ownership” and “no hidden allocations.”

**Alternatives considered**: (1) Context stores a pointer — rejected because app would have to keep theme valid for context lifetime. (2) Context allocates a copy on heap — rejected to avoid allocation in setter and to keep context layout simple.

---

### Where the theme lives and how it is read

**Decision**: Add a `cui_theme` (or `struct cui_theme`) field to `struct cui_ctx` in `context.c`. Initialize it in `cui_create` to the built-in default (same values as current `theme.h` #defines). Provide an internal accessor (e.g. `const cui_theme *cui_ctx_theme(cui_ctx *ctx)` in `context.h`) so `draw_cmd.c` and `layout.c` can read the current theme without reaching into the opaque context. Draw and layout code use this accessor instead of `CUI_THEME_*` macros for runtime values.

**Rationale**: Single source of truth; no global state; clear data flow (context → draw/layout). Internal accessor keeps core and layout layers consistent.

**Alternatives considered**: Keeping macros and overriding via a “theme override” pointer — rejected in favor of one canonical theme in context for simplicity.

---

### Built-in default and theme.h

**Decision**: Define a static const default theme (same values as current `theme.h`) in `context.c` or in `theme.c` (if we add a small theme.c). On `cui_create`, copy that default into `ctx->theme`. Keep `theme.h` for internal use: it can define the default initializer or the default struct, and can still #define the numeric values for the default so that C files can initialize the default struct without duplicating literals. Alternatively, remove the #defines and use a single `cui_theme cui_theme_default(void)` or `static const cui_theme` in one place.

**Rationale**: One place for default values; `theme.h` remains the logical home for “default theme” constants or initializer.

**Alternatives considered**: Leaving all #defines and having draw_cmd read from context only when “theme overridden” — rejected to have a single code path (always read from context).

---

### Dark theme preset

**Decision**: Provide a function `void cui_theme_dark(cui_theme *out)` (or `cui_theme cui_theme_dark(void)` returning by value) that fills a theme struct with dark colors (e.g. dark gray/black backgrounds, light text, same focus ring semantics). App does: `cui_theme t; cui_theme_dark(&t); cui_set_theme(ctx, &t);`. Alternatively a named constant `extern const cui_theme CUI_THEME_DARK` if C allows const struct in header. Prefer function to avoid static storage and to allow future params (e.g. accent color).

**Rationale**: One-call dark mode; no new API beyond theme struct + setter + one preset getter. Function returning by value or filling pointer keeps ABI and init simple.

**Alternatives considered**: Multiple presets (e.g. “high contrast”) — deferred to later; 0.6.0 ships one dark preset only.

---

### Backward compatibility and macros

**Decision**: After 0.6.0, `CUI_THEME_*` macros in `theme.h` may remain as the default theme’s field values (for internal init of the default struct) but must not be used in draw_cmd.c or layout.c at runtime; those files use the context’s theme. Public API does not expose the macros as the way to “set” theme; they are implementation details of the default. Document that the default theme is equivalent to the previous compile-time values.

**Rationale**: No breaking change for apps that never touched theme.h; default behavior is unchanged. Clear migration for apps that want runtime theme: use `cui_set_theme`.
