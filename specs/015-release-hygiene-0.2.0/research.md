# Research: Release Hygiene (0.2.0)

Decisions and rationale for documentation, CI, and tooling. No unknowns remained in Technical Context; this file records choices for CHANGELOG, Windows CI, sanitizers, and formatting.

---

## 1. CHANGELOG format

**Decision**: Adopt [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) with sections Added, Changed, Fixed, Deprecated, Removed, Security. Use `[X.Y.Z] - YYYY-MM-DD` version headings and an Unreleased section at the top.

**Rationale**: Widely recognized, human- and tool-friendly; aligns with semantic versioning already used by the project.

**Alternatives considered**: Custom format (rejected — no benefit over standard); machine-only format (rejected — maintainers need readable history).

---

## 2. Windows (MSVC) on GitHub Actions

**Decision**: Add a Windows job using `windows-latest` (or `windows-2022`). Use the MSVC toolchain from the runner (e.g. open Developer Command Prompt or `msbuild`/`cl` from Visual Studio). If the project uses Make, either: (a) use a Windows port of make (e.g. from Chocolatey, or Make from Visual Studio build tools), or (b) add a small script that invokes `cl` and `link` with equivalent flags (`/nologo`, `/W4`, `/std:c11` or equivalent, `/Fe` for output). Prefer (a) if `make` is available on the runner to keep one build path.

**Rationale**: README claims “Compiles with gcc, clang, or MSVC”; CI must validate that. Windows runners have MSVC installed; no need for a specific MSVC version for 0.2.0.

**Alternatives considered**: MinGW on Windows (rejected for this milestone — we want to assert MSVC specifically); skipping Windows (rejected — would leave the README claim untested).

---

## 3. ASan and UBSan build targets

**Decision**: Add Make targets (e.g. `asan`, `ubsan` or a single `sanitizers` target) that set `CFLAGS` and `LDFLAGS` with `-fsanitize=address` and `-fsanitize=undefined`, plus `-fno-omit-frame-pointer` and `-g`. Use the same sanitizer flags at link time. Run the existing unit and integration test binaries under these builds. Do not use `-static-libasan` (problematic on macOS). Prefer separate targets for ASan and UBSan so developers can run one at a time; CI can run both (e.g. two jobs or two steps).

**Rationale**: Catches memory and undefined-behavior bugs; GCC and Clang both support these flags; Filament and other C/C++ projects use this pattern.

**Alternatives considered**: Single combined ASan+UBSan target only (acceptable if CI runs it; we still document both for local use); CMake-only sanitizer (rejected — project uses Make).

---

## 4. .clang-format

**Decision**: Add a `.clang-format` at repo root with a C-oriented style (e.g. `BasedOnStyle: LLVM` or `Linux` with `Language: Cpp`/C settings). Use 4-space indent and column limit (e.g. 100 or 120) to match typical ClearUI style. Document in README or CONTRIBUTING that contributors can run `clang-format -i` on changed files (or the whole tree) and that CI may optionally check formatting.

**Rationale**: Reduces style drift and review noise; LLVM/Linux styles are common for C and work with `clang-format` out of the box.

**Alternatives considered**: No formatter (rejected — spec requires style enforcement); editorconfig only (can coexist; .clang-format is the primary contract for C formatting).
