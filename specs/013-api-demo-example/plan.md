# Implementation Plan: Full API Demo Example

**Branch**: `013-api-demo-example` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/013-api-demo-example/spec.md`

## Summary

Add a single comprehensive example file (`examples/demo.c`) that exercises the entire ClearUI public API with developer- and AI-agent-friendly comments. The file runs headlessly via stub platform + software RDI, prints a pass/fail summary, and is built with `make demo`. No new library code is required — this is purely a documentation/example artifact.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)
**Primary Dependencies**: None (zero external dependencies; uses built-in stub platform + software RDI)
**Storage**: N/A
**Testing**: The demo itself is self-verifying (assertions + printed output). Also added to CI validation.
**Target Platform**: Any platform with a C11 compiler (Linux, macOS, Windows)
**Project Type**: Library (this feature adds an example, not library code)
**Performance Goals**: N/A (example code, not production)
**Constraints**: Under 200 lines. Must call every public API function (except `cui_run`, `cui_dev_overlay`).
**Scale/Scope**: 1 new file, 2 modified files

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: Demo code reads as pseudocode with explanatory comments. No macro magic.
- [x] **Ownership**: Memory model respected. `cui_state` for persistent data, `cui_frame_printf`/`cui_frame_alloc` for transient, arena-managed UI nodes never freed.
- [x] **Zero-to-Window**: `make demo && ./demo` — single command, under 5 seconds.
- [x] **C99/C11**: Pure C11, no extensions, no new dependencies.
- [x] **Performance**: N/A (example, not library code).
- [x] **Beautiful & Accessible**: Demo demonstrates `cui_aria_label`, `cui_tab_index`, and keyboard navigation.
- [x] **Tech constraints**: No new API functions. API surface unchanged. RDI/platform abstraction demonstrated correctly.
- [x] **Layers**: Demo only uses Widget and Layout layers (top two). Platform/RDI via interfaces.

## Project Structure

### Documentation (this feature)

```text
specs/013-api-demo-example/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0: API enumeration, comment strategy, headless approach
├── data-model.md        # Phase 1: Demo state variables and relationships
└── quickstart.md        # Phase 1: Build & run instructions
```

### Source Code (repository root)

```text
examples/
└── demo.c               # NEW: Full API demo (~150-200 lines)

Makefile                  # MODIFIED: Add `demo` target
README.md                # MODIFIED: Add "Examples" section
```

**Structure Decision**: `examples/` is a new top-level directory, standard for library projects. One file only — keeping it simple and discoverable.

## Implementation Phases

### Phase 1: Create `examples/demo.c`

Write the demo file organized into clearly commented sections:

1. **Header** — file purpose, build command, API coverage summary
2. **Includes** — `clearui.h`, `clearui_platform.h`, `clearui_rdi.h`, `<stdio.h>`, `<assert.h>`, `<string.h>`
3. **main()** — organized as:
   - Context creation (`cui_create` with `cui_config`)
   - Platform + RDI wiring (`cui_set_platform`, `cui_set_rdi`)
   - Frame 1: Full UI build
     - State (`cui_state`, `cui_frame_printf`, `cui_frame_alloc`)
     - Layout nesting (`cui_center` > `cui_column` > `cui_row`, plus `cui_scroll`, `cui_wrap`, `cui_stack`)
     - All widgets (`cui_label`, `cui_label_styled`, `cui_button`, `cui_checkbox`, `cui_icon_button`, `cui_text_input`, `cui_spacer`)
     - Canvas (`cui_canvas`, `cui_draw_rect`, `cui_draw_circle`, `cui_draw_text`)
     - Styling (`cui_push_style`, `cui_pop_style`)
     - Accessibility (`cui_aria_label`, `cui_tab_index`)
   - Frame 2: Input injection
     - `cui_inject_click` on the "+" button
     - `cui_inject_key(CUI_KEY_TAB)` for focus
     - Rebuild UI, observe `cui_button` returning 1
   - Frame 3: State verification
     - Assert counter incremented
     - Print summary
   - Cleanup (`cui_destroy`, RDI shutdown, platform destroy)

### Phase 2: Build integration

1. Add `demo` target to `Makefile` (links `examples/demo.c` against `$(OBJS)`)
2. Add `demo` to `clean` rule
3. Add `demo` to `.PHONY`

### Phase 3: README update

1. Add "Examples" section to `README.md` between "Build" and "Project Structure"
2. Link to `examples/demo.c` with description and build command

### Phase 4: Validation

1. `make demo && ./demo` — exit code 0
2. `make clean && make all && make unit-tests && make integration-tests && make demo` — full pipeline
3. Zero warnings under `-std=c11 -Wall -Wextra -Wpedantic`

## Complexity Tracking

No constitution violations. No complexity justifications needed.
