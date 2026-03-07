# Feature Spec: Add Default Alignment to Arena Allocator

**Branch**: `003-arena-alignment` | **Date**: 2026-03-07

## Problem

`cui_arena_alloc` performs no alignment — it bumps the internal pointer by exactly the requested size. All `cui_node` structs (containing `float`, pointers, `size_t`) are allocated through this function, producing potentially unaligned memory. This causes:

- **Undefined behavior** on strict-alignment architectures (ARM, SPARC)
- **Performance penalties** on x86/x64 (unaligned loads are slower even when supported)
- **Correctness risk** when storing types that require natural alignment (pointers, doubles)

`cui_arena_alloc_aligned` exists as a separate function but is never called anywhere in the codebase — it is dead code.

## Requirements

1. **R1**: `cui_arena_alloc` MUST return memory aligned to at least `_Alignof(max_align_t)` (C11), ensuring safety for any standard type.
2. **R2**: The public API signature of `cui_arena_alloc` MUST NOT change (no ABI break).
3. **R3**: `cui_arena_alloc_aligned` MUST remain available for callers needing custom alignment (e.g., SIMD vectors).
4. **R4**: Existing tests MUST continue to pass. New alignment-specific tests MUST be added.
5. **R5**: Arena memory overhead from alignment padding MUST be minimal (worst case: `_Alignof(max_align_t) - 1` bytes per allocation).

## Scope

- **In scope**: `src/core/arena.c`, `src/core/arena.h`, `tests/unit/test_arena.c`
- **Out of scope**: Frame allocator alignment (separate concern, same pattern can be applied later)

## Acceptance Criteria

- All pointers returned by `cui_arena_alloc` are aligned to `_Alignof(max_align_t)`.
- `((uintptr_t)ptr % _Alignof(max_align_t)) == 0` for every returned pointer.
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
- New test in `test_arena.c` explicitly verifies alignment of returned pointers.
