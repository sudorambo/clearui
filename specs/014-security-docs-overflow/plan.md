# Implementation Plan: Security and Robustness Hardening (Remaining Recommendations)

**Branch**: `014-security-docs-overflow` | **Date**: 2026-03-07 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/014-security-docs-overflow/spec.md`

## Summary

Address the remaining recommendations from the deep memory/security review: (1) document that `cui_frame_printf`'s `fmt` must be application-controlled and optionally cap formatted length to avoid format-string and DoS risk; (2) document that UTF-8 parameters to font/measure APIs must be NUL-terminated; (3) add integer-overflow checks in arena and frame allocator grow paths; (4) document that vault keys must be valid C strings; (5) replace `strcpy` with a bounded alternative in test_vault.c. Implementation is documentation-first with targeted code hardening (overflow checks, test fix).

## Technical Context

**Language/Version**: C11 (`-std=c11 -Wall -Wextra -Wpedantic`)  
**Primary Dependencies**: None (stdlib only; optional vendored stb_truetype for font)  
**Storage**: N/A  
**Testing**: Plain C with `assert.h`, Makefile targets (`make unit-tests`, `make integration-tests`)  
**Target Platform**: Linux, macOS (CI); same as existing ClearUI targets  
**Project Type**: Library (C11 declarative immediate-mode UI)  
**Performance Goals**: 60fps; no new hot paths; overflow checks are O(1) branches  
**Constraints**: Zero new external deps; API surface unchanged (documentation and internal hardening only)  
**Scale/Scope**: ~6 files touched (context.c, atlas.c, arena.c, frame_alloc.c, vault.c or docs, test_vault.c); no new public API

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify against `.specify/memory/constitution.md`:

- [x] **Clarity**: Documentation and overflow checks improve clarity of contracts; no hidden behavior.
- [x] **Ownership**: Memory model unchanged; no new allocators or free() on UI nodes.
- [x] **Zero-to-Window**: Build and first-run unchanged; docs and internal checks only.
- [x] **C99/C11**: All changes are standard C11; overflow checks use `SIZE_MAX` from `<stdint.h>`.
- [x] **Performance**: Overflow checks are single comparisons; no impact on 60fps target.
- [x] **Beautiful & Accessible**: No change to defaults or a11y.
- [x] **Tech constraints**: No new public API; RDI/platform unchanged; well under API cap.
- [x] **Layers**: Changes are in Core (arena, frame_alloc, context, vault) and font (atlas); tests in tests/unit. Dependencies remain downward only.

## Project Structure

### Documentation (this feature)

```text
specs/014-security-docs-overflow/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output (contracts / doc only; no new entities)
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (API contracts for frame_printf, vault key, utf8)
└── tasks.md             # Phase 2 output (/speckit.tasks — not created by /speckit.plan)
```

### Source Code (repository root)

```text
src/
├── core/
│   ├── context.c        # Document frame_printf; optionally cap n (e.g. max 64K)
│   ├── arena.c          # Overflow check before new_cap *= 2
│   ├── frame_alloc.c    # Overflow check before new_cap *= 2
│   └── vault.c         # Document key NUL-termination (or add comment in header/spec)
├── font/
│   └── atlas.c          # Document utf8_next / utf8 NUL-termination
include/
└── clearui.h            # Optional: brief doc for cui_frame_printf, cui_state/vault key

tests/
└── unit/
    └── test_vault.c     # Replace strcpy with snprintf for bounded copy
```

**Structure Decision**: Single project; existing `src/` + `tests/` layout. No new modules; documentation and small code edits only.

## Complexity Tracking

No constitution violations. All changes are documentation and defensive overflow checks within existing architecture.
