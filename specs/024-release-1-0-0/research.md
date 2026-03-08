# Research: 1.0.0 Release

## R1: Cross-platform CI coverage

**Decision**: CI already runs on Ubuntu and macOS (`build-and-test` matrix) and Windows (MSYS2/UCRT64). ASan, UBSan, and LeakSanitizer run on Ubuntu. This is sufficient for Tier 1.

**Rationale**: Existing `.github/workflows/ci.yml` covers all three platforms and sanitizers. No additional CI configuration needed beyond ensuring the branch triggers workflows.

**Alternatives considered**: Adding `windows-latest` to the sanitizer matrix (rejected: sanitizers not well-supported under MSYS2/MinGW).

---

## R2: CHANGELOG completeness

**Decision**: Verify that `CHANGELOG.md` has entries for all milestones: 0.2.0, 0.3.0, 0.4.0, 0.5.0, 0.6.0, 0.7.0, 0.8.0, 0.9.0, 0.10.0. Add a 1.0.0 entry documenting the version bump and release.

**Rationale**: ROADMAP requires "CHANGELOG.md covers every milestone."

---

## R3: Demo with visible window

**Decision**: `examples/demo.c` already exercises all public APIs headlessly (stub platform + software RDI). For the "visible window" checklist item, build with `WITH_SDL3=1` and verify the SDL3 adapter + software RDI renders. The demo runs 3 frames then exits; this is sufficient.

**Rationale**: SDL3 adapter and software RDI ship; demo wires them. Manual verification or CI with display confirms rendering.

**Alternatives considered**: Adding a screenshot comparison (too complex for 1.0; can be a follow-up).

---

## R4: GitHub release

**Decision**: Tag `v1.0.0` on the main branch after merging all milestone branches. Create a GitHub release via `gh release create v1.0.0 --title "ClearUI 1.0.0" --notes-file CHANGELOG_SECTION.md` (or equivalent). Attach `libclearui.a` if desired.

**Rationale**: Standard release process; `gh` CLI is available in CI and locally.

---

## R5: Version bump

**Decision**: Set `CUI_VERSION_MAJOR=1, MINOR=0, PATCH=0` in `include/clearui.h`. `cui_version_string()` will then return `"1.0.0"` automatically.

**Rationale**: Single source of truth in the macros; `version.c` derives the string.
