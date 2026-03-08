# Feature Specification: 1.0.0 Release

**Version**: 1.0.0  
**Branch**: `024-release-1-0-0`

## Goal

Verify that all milestones 0.2.0–0.10.0 are complete, run the release checklist (cross-platform tests, sanitizer clean, changelog coverage, README accuracy, working demo, API docs published), bump version to 1.0.0, tag, and create a GitHub release.

## Checklist (from ROADMAP)

1. All unit and integration tests pass on Linux, macOS, and Windows.
2. ASan, UBSan, and Valgrind/LeakSanitizer clean.
3. CHANGELOG.md covers every milestone (0.2.0–0.10.0).
4. README updated with final API surface.
5. At least one real platform backend ships (SDL3).
6. At least one real render driver ships (software framebuffer).
7. `examples/demo.c` runs with a visible window and renders correctly.
8. API reference documentation published (`docs/API.md`).
9. Tag `v1.0.0`, create GitHub release.

## User Stories

1. **US1** As a maintainer, I want all CI checks (build, unit, integration, sanitizers, leak-check) to pass on all tier-1 platforms so the release is trustworthy.
2. **US2** As an adopter, I want the CHANGELOG and README to be complete and accurate so I can evaluate the library.
3. **US3** As an adopter, I want `examples/demo.c` to run with a visible window (SDL3 + software RDI) and render correctly so I can see ClearUI in action.
4. **US4** As a maintainer, I want the version set to 1.0.0, a git tag `v1.0.0`, and a GitHub release so the stable release is discoverable.

## Acceptance Criteria

- `make all && make unit-tests && make integration-tests` passes on Ubuntu, macOS, Windows (CI).
- `make asan` and `make ubsan` pass on Ubuntu.
- `make leak-check-lsan` passes on Ubuntu.
- CHANGELOG.md has entries for 0.2.0, 0.3.0, 0.4.0, 0.5.0, 0.6.0, 0.7.0, 0.8.0, 0.9.0, 0.10.0, and 1.0.0.
- README lists the final public API surface and docs links.
- `make demo && ./demo` exits 0 (headless). With SDL3: `make demo WITH_SDL3=1 && ./demo` opens a window and renders.
- `docs/API.md` and `docs/MIGRATION.md` exist and are current.
- `include/clearui.h` has `CUI_VERSION_MAJOR=1, MINOR=0, PATCH=0`.
- Git tag `v1.0.0` created; GitHub release drafted.

## Out of Scope

- New features; this is a release-only milestone.
- GPU render driver; software RDI is sufficient for 1.0.
- Tier-2 platform testing (iOS, Android, Web).

## Requirements

- **R1** No code changes except version bump, CI updates, docs, and demo fixes.
- **R2** API must remain frozen (per 0.10.0 commitment).
