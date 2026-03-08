# Research: API Polish (0.10.0)

## R1: cui_layout.padding vs padding_x / padding_y

**Decision**: **Clarify, do not remove.** Keep all three fields. Document in the header and README: "If `padding_x` or `padding_y` is > 0, they override; otherwise `padding` sets both. Zero-init `(cui_layout){}` means no padding." No deprecation for 0.10.0 to avoid breaking existing code before freeze; deprecation can be a 1.x option if desired.

**Rationale**: Removing or deprecating `padding` would break callers using it. Clarification satisfies "resolve ambiguity" without breaking change.

**Alternatives considered**: Deprecate `padding` (rejected for 0.10.0 to minimize churn before freeze). Remove `padding` (breaking).

---

## R2: cui_version_string()

**Decision**: Add `const char *cui_version_string(void);` implemented in core (e.g. `context.c` or a small `version.c`). Return a string literal or static buffer built from `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH` so it stays in sync. No allocation.

**Rationale**: Single function, no new dependency; matches common C library practice.

**Alternatives considered**: Macro-only (no runtime string; rejected for "runtime" requirement). Snprintf in a static buffer (acceptable if no thread-safety requirement; document as not thread-safe).

---

## R3: API reference (Doxygen vs custom)

**Decision**: **Doxygen** with a minimal `Doxyfile` and comments in public headers. Generate to `docs/api/` or `api/` in repo; add to `.gitignore` if generated in CI only, or commit generated HTML for GitHub Pages. Alternative: a single Markdown file listing all public APIs (custom script or hand-maintained) if Doxygen is deemed too heavy.

**Rationale**: Doxygen is widely used for C; minimal comments (brief + params) suffice. Keeps API reference maintainable as the API evolves.

**Alternatives considered**: Custom generator (more work). Hand-written Markdown (can go out of date).

---

## R4: Migration guide

**Decision**: Add `docs/MIGRATION.md` (or a **Migration** section in README) listing breaking or notable changes by version (0.2.0 through 0.10.0): e.g. poll_events signature change, layout_run(ctx), theme, error callback, etc. One subsection per version; bullet list of changes and "Before/After" or "Upgrade" notes.

**Rationale**: Gives adopters a single place to check when upgrading; supports "migration guide for any API changes since 0.1.0".

**Alternatives considered**: CHANGELOG only (already exists; migration guide is more upgrade-focused).

---

## R5: cui_image, cui_tooltip, cui_popup

**Decision**: **Consider** only: include in spec as optional. If time and API cap allow: `cui_image(ctx, id, texture_or_handle, w, h)` (or bounds in opts) and either `cui_tooltip(ctx, "text")` or `cui_popup(ctx, opts)` as a container. Defer to task breakdown: implement only if prioritized and under function count. Otherwise document as "reserved for 1.x" in the spec.

**Rationale**: Roadmap says "Consider adding"; constitution caps API size. Making them optional in the plan keeps 0.10.0 achievable without blocking on new widgets.

**Alternatives considered**: Mandate both (risks scope creep). Drop entirely (spec says "consider", so keep as optional).

---

## R6: API freeze

**Decision**: After 0.10.0, document in README and/or CONTRIBUTING: "Public API is frozen; no breaking changes. New APIs must be additive only until 1.0." No code change; process and documentation only.

**Rationale**: Freeze is a policy; documenting it makes the contract clear.

**Alternatives considered**: Machine-checked API stability (e.g. script comparing exported symbols) — can be a follow-up.
