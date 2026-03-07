# Implementation Plan: Text Input & Keyboard (0.3.0)

**Branch**: `016-text-input-keyboard-0.3.0` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/016-text-input-keyboard-0.3.0/spec.md`

## Summary

Milestone 2 completes the text input widget: wire keyboard character input via a new API (e.g. `cui_inject_char`), implement cursor position tracking in retained state, support backspace/delete via key codes or the same path, make `cui_text_input` return 1 when the buffer content changes, and add unit and integration tests. The widget already exists and draws the buffer; this adds editing behavior and retained cursor.

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None (stdlib only; optional vendored stb_truetype for font)  
**Storage**: Retained UI tree (context) holds cursor index per text input; buffer is caller-owned, content in app memory  
**Testing**: Existing `make unit-tests` and `make integration-tests`; add unit tests for text input editing and one integration test (type into field, read back buffer)  
**Target Platform**: Tier 1 — Windows, macOS, Linux (same as today)  
**Project Type**: Library (declarative immediate-mode GUI)  
**Performance Goals**: 60fps; text editing is per-frame, no new heavy work  
**Constraints**: Single-threaded; cursor and “changed” state must survive frame boundary via retained tree or context  
**Scale/Scope**: One new public API (e.g. `cui_inject_char`), 2–3 new key codes (backspace, delete, optionally left/right arrow); ~2–4 new tests  

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: No hidden allocations, macro magic, or implicit global state; API readable as pseudocode. (`cui_inject_char` and key codes are explicit.)
- [x] **Ownership**: Memory model respected; buffer remains caller-owned; cursor in retained node or context, no `free()` on UI nodes.
- [x] **Zero-to-Window**: Build and first-run unchanged; new code is additive.
- [x] **C99/C11**: No C++; new API is C11.
- [x] **Performance**: 60fps target unchanged; edit logic is O(length) per frame when focused.
- [x] **Beautiful & Accessible**: Text input already in a11y tree; keyboard input improves accessibility.
- [x] **Tech constraints**: One new function + key codes; API surface stays within cap.
- [x] **Layers**: Widget layer (text_input.c) and core (context.c) only; dependencies downward.

## Project Structure

### Documentation (this feature)

```text
specs/016-text-input-keyboard-0.3.0/
├── plan.md              # This file
├── research.md         # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (API contract for inject_char, return value)
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
include/
  clearui.h              # add cui_inject_char; add CUI_KEY_BACKSPACE, CUI_KEY_DELETE (and optionally CUI_KEY_LEFT, CUI_KEY_RIGHT)

src/
  core/
    context.c            # process pending_key: when focused is text input, handle backspace/delete; accept pending_char for insert
    node.h               # add cursor index to node (or store in context keyed by id)
  widget/
    text_input.c         # register focus; return 1 when buffer changed (compare or set dirty flag)

tests/
  unit/
    test_text_input.c    # new: insert, backspace, delete, cursor, full buffer
  integration/
    test_text_input_edit.c  # new: type into field, read back buffer
```

**Structure Decision**: Single-project layout. Cursor can live in retained node (add `text_input_cursor` to `cui_node`) so diff preserves it; “changed” can be a per-frame flag set when insert/delete runs, read by `cui_text_input` on the next frame or via a return-path from the processing that mutates the buffer.

## Complexity Tracking

No constitution violations. Cursor and changed state are minimal additions; buffer remains caller-owned.
