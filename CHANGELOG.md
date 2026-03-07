# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2026-03-07

### Added

- `CHANGELOG.md` in [Keep a Changelog](https://keepachangelog.com/) format.
- Documentation in `include/clearui.h` and README for color format (`0xAARRGGBB`), single-threaded contract, and fixed limits (silent truncation).
- Windows job in CI (MSYS2/MinGW); CI runs on Ubuntu, macOS, and Windows.
- ASan and UBSan Makefile targets (`make asan`, `make ubsan`) and a CI job that runs tests under sanitizers.
- `.clang-format` at repo root; Contributing section documents sanitizers and formatting.

### Changed

### Fixed

### Deprecated

### Removed

### Security
