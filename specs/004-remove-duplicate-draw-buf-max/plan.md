# Implementation Plan: Remove Duplicate CUI_DRAW_BUF_MAX in render.c

**Branch**: `004-remove-duplicate-draw-buf-max` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/004-remove-duplicate-draw-buf-max/spec.md`

## Summary

`src/core/render.c` redefines `CUI_DRAW_BUF_MAX` (already defined in `draw_cmd.h` which it includes). Remove the duplicate to eliminate divergence risk. One-line deletion, no behavioral change.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None
**Testing**: `make unit-tests && make integration-tests`
**Target Platform**: Linux, macOS (CI)
**Project Type**: Library (C GUI library)
**Scale/Scope**: 1 line deleted from 1 file

## Constitution Check

- [x] **Clarity**: No API change. Removing duplication improves clarity.
- [x] **Ownership**: No memory model change.
- [x] **Zero-to-Window**: No build change.
- [x] **C99/C11**: No language feature change.
- [x] **Performance**: No runtime change.
- [x] **Beautiful & Accessible**: N/A.
- [x] **Tech constraints**: No API surface change.
- [x] **Layers**: Change within Core layer only.

**Result**: All gates pass.

## Project Structure

```text
src/core/render.c    # Modified: remove duplicate #define
```

## Complexity Tracking

No violations. Table not needed.
