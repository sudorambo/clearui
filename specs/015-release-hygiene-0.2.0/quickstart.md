# Quickstart: Release Hygiene (0.2.0)

Short guide for contributors and maintainers after 0.2.0 tooling and docs are in place.

## Running tests

```bash
make clean && make all && make unit-tests && make integration-tests
```

## Sanitizers (after 0.2.0)

Run tests under AddressSanitizer and UndefinedBehaviorSanitizer (exact targets may be `make asan` / `make ubsan` or combined):

```bash
make asan    # build and run tests with ASan
make ubsan   # build and run tests with UBSan
```

Use these before submitting PRs when touching memory or hot paths. CI will run at least one sanitizer build.

## Formatting (after 0.2.0)

Format C sources with the project’s `.clang-format`:

```bash
clang-format -i path/to/file.c
# or format the whole tree:
find src tests examples -name '*.c' -exec clang-format -i {} \;
```

Check README or CONTRIBUTING for any CI check that enforces formatting.

## Updating the CHANGELOG

When cutting a release (e.g. 0.2.0):

1. Under **Unreleased**, move or copy items into a new version block.
2. Add a heading: `## [X.Y.Z] - YYYY-MM-DD` (date of release).
3. Keep sections: Added, Changed, Fixed, Deprecated, Removed, Security.
4. Leave an empty **Unreleased** section at the top for the next cycle.

Example:

```markdown
# Changelog

## [Unreleased]

## [0.2.0] - 2026-03-07

### Added
- CHANGELOG and Keep a Changelog format.
- Documentation for color format, single-threaded contract, and fixed limits.
- Windows (MSVC) in CI; ASan/UBSan targets and CI step.
- `.clang-format` for style.
```

## Where things live

| Item              | Location |
|-------------------|----------|
| Public API        | `include/clearui.h` |
| Color/thread/limits | In header and README |
| Changelog         | `CHANGELOG.md` |
| CI                | `.github/workflows/ci.yml` |
| Format config     | `.clang-format` |
| Sanitizer targets | Makefile |
