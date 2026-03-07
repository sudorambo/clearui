# Feature Specification: Release Hygiene (0.2.0)

**Feature Branch**: `015-release-hygiene-0.2.0`  
**Created**: 2026-03-07  
**Status**: Draft  
**Input**: ROADMAP.md Milestone 1 — foundational items every release needs; no functional changes.

## User Scenarios & Testing

### User Story 1 - Maintainer ships 0.2.0 with documented contracts (Priority: P1)

As a maintainer I can tag 0.2.0 with a CHANGELOG, documented color/threading/limits, and CI that includes Windows (MSVC) and sanitizers so that users and contributors have clear expectations and the project meets its advertised support.

**Why this priority**: Unblocks confident releases and contributor onboarding.

**Independent Test**: Run full CI (Linux, macOS, Windows); open CHANGELOG and public header and verify color format, single-threaded contract, and fixed limits are documented.

**Acceptance Scenarios**:

1. **Given** the repo, **When** I read `include/clearui.h` and README, **Then** color format `0xAARRGGBB`, single-threaded contract, and fixed limits (with silent-truncation behavior) are clearly stated.
2. **Given** a release tag, **When** I open `CHANGELOG.md`, **Then** it follows [Keep a Changelog](https://keepachangelog.com/) and contains an entry for 0.2.0.

---

### User Story 2 - Contributor runs sanitizers and format (Priority: P2)

As a contributor I can run ASan/UBSan builds and format code with `.clang-format` so that I catch memory/undefined issues and match project style before submitting.

**Why this priority**: Reduces regressions and keeps style consistent.

**Independent Test**: `make asan` (or equivalent) and `make ubsan` run tests; `clang-format -i` uses repo `.clang-format` and produces no unrelated diff.

**Acceptance Scenarios**:

1. **Given** the repo, **When** I run the documented sanitizer target(s), **Then** unit and integration tests run under ASan and UBSan without errors.
2. **Given** the repo, **When** I run the formatter per CONTRIBUTING/README, **Then** existing code passes and new code can be formatted consistently.

---

### User Story 3 - CI includes Windows (MSVC) (Priority: P2)

As a maintainer I have Windows (MSVC) in the GitHub Actions matrix so that README’s claim of MSVC support is validated on every push.

**Why this priority**: Avoids broken “we support MSVC” without actually testing it.

**Independent Test**: Push a change and confirm the Windows job runs `make all` and tests (or equivalent) and succeeds.

**Acceptance Scenarios**:

1. **Given** the workflow file, **When** CI runs on a PR, **Then** a Windows (MSVC) job is present and green.
2. **Given** README says “Compiles with gcc, clang, or MSVC”, **Then** CI exercises at least one MSVC build.

---

### Edge Cases

- What happens when CHANGELOG is missing for a new release? Process doc (quickstart or CONTRIBUTING) should require updating CHANGELOG before tagging.
- How does the project handle a contributor without clang? `.clang-format` is optional tooling; format can be enforced in CI and documented as “recommended” for local use.

## Requirements

### Functional Requirements

- **FR-001**: Project MUST provide `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH` in `clearui.h` (already present; confirm for 0.2.0).
- **FR-002**: Project MUST include `CHANGELOG.md` in [Keep a Changelog](https://keepachangelog.com/) format.
- **FR-003**: Public header and README MUST document color format `0xAARRGGBB`.
- **FR-004**: Public header and README MUST document single-threaded contract (all API calls on one thread).
- **FR-005**: Public header and README MUST document fixed limits and silent-truncation behavior:
  - `CUI_PARENT_STACK_MAX` (16) — max container nesting depth
  - `CUI_FOCUSABLE_MAX` (64) — max focusable widgets per frame
  - `CUI_A11Y_MAX` (128) — max accessibility entries
  - `CUI_LAST_CLICKED_ID_MAX` (64) — max widget ID length
  - `CUI_FRAME_PRINTF_MAX` (65536) — max formatted string output
- **FR-006**: GitHub Actions CI MUST include a Windows (MSVC) build and test job.
- **FR-007**: Makefile (or equivalent) MUST provide ASan and UBSan build/test targets; CI MUST run them.
- **FR-008**: Repository MUST include `.clang-format` for consistent style; contribution docs MUST describe how to format.

### Key Entities

- **CHANGELOG.md**: Versioned list of Added/Changed/Fixed/Deprecated/Removed/Security per Keep a Changelog.
- **Version macros**: `CUI_VERSION_MAJOR`, `CUI_VERSION_MINOR`, `CUI_VERSION_PATCH` in `clearui.h`.
- **Documented limits**: The five constants above; behavior is silent truncation when exceeded.

## Success Criteria

- **SC-001**: A new contributor can find color format, threading model, and limits in the public header or README within one minute.
- **SC-002**: CI runs on every push/PR on Ubuntu, macOS, and Windows (MSVC), and includes at least one sanitizer run (ASan or UBSan).
- **SC-003**: Running the documented format command produces no unexpected changes on the current codebase (or only whitespace/formatting that matches `.clang-format`).
