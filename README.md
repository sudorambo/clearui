# ClearUI

C99/C11 GUI library with a **Declarative Immediate** model: you write immediate-mode–style code each frame while the library keeps an internal retained tree for layout, diffing, and accessibility.

## Build

From the repo root:

```bash
make all          # build library objects
make unit-tests    # arena, vault, layout
make integration-tests   # Hello World + Counter pipeline
```

**Single-command validation (quickstart):**

- Hello World: `make test_hello && ./test_hello`
- Counter: `make test_counter && ./test_counter`

Requires a C11 compiler (`cc`, `gcc`, or `clang`). No external dependencies for the default build (stub platform and software RDI).

## Quickstart

See **[specs/001-clearui-core/quickstart.md](specs/001-clearui-core/quickstart.md)** for:

- Hello World and Counter examples
- Single-command build (with amalgamation or precompiled lib)
- Layout, widgets, style, and distribution notes

## Layout

- **Include**: `include/clearui.h`, `include/clearui_rdi.h`, `include/clearui_platform.h`
- **Source**: `src/core/`, `src/layout/`, `src/widget/`, `src/rdi/`, `src/platform/`, `src/font/`
- **Tests**: `tests/unit/`, `tests/integration/`

## CI

GitHub Actions runs `make all`, `make unit-tests`, and `make integration-tests` on **Ubuntu** and **macOS** on every push to `main` and `001-clearui-core`. See [.github/workflows/ci.yml](.github/workflows/ci.yml). Windows can be added when a Windows build (e.g. MinGW or MSVC) is configured.

## Distribution

- **Development**: use the Makefile and include paths above.
- **Two-file drop-in**: an amalgamated `clearui.c` (all sources in one file) for `clearui.h` + `clearui.c` is a follow-up per spec §7.1. Until then, link the built objects or build a static library from `src/`.

## License and spec

See the project spec and constitution under `specs/001-clearui-core/` and `.specify/`.
