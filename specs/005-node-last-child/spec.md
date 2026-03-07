# Feature Spec: Add `last_child` to `cui_node` for O(1) Appends

**Branch**: `005-node-last-child` | **Date**: 2026-03-07

## Problem

`cui_node_append_child` currently walks the entire sibling list to find the tail before appending:

```c
cui_node *p = parent->first_child;
while (p->next_sibling) p = p->next_sibling;
p->next_sibling = child;
```

This is O(n) per append, making tree construction O(n²) for n children. For the 60fps target on trees with hundreds of children (e.g. list views, tables), this is a measurable bottleneck in the frame-build phase.

## Requirements

1. **R1**: Add a `cui_node *last_child` pointer to `struct cui_node` in `src/core/node.h`.
2. **R2**: Update `cui_node_append_child` in `src/core/node.c` to maintain `last_child`, achieving O(1) appends.
3. **R3**: `cui_node_alloc` must zero-initialize `last_child` (already covered by `memset`).
4. **R4**: All tree traversals that only read `first_child`/`next_sibling` MUST remain unchanged — `last_child` is write-side only.
5. **R5**: All existing tests MUST continue to pass.
6. **R6**: Add a unit test verifying O(1) append correctness (parent→last_child == last appended child).

## Scope

- **In scope**: `node.h` (struct field), `node.c` (append logic), `test_layout.c` (new test section).
- **Out of scope**: Using `last_child` for reverse iteration, changing layout traversal order, adding `prev_sibling`.

## Acceptance Criteria

- `cui_node_append_child` does not contain a `while` loop.
- `parent->last_child` points to the most recently appended child after every append.
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings.
