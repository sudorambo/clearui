# Research: Add `last_child` to `cui_node` for O(1) Appends

## R1: Intrusive linked-list tail-pointer pattern

**Decision**: Add a `cui_node *last_child` field directly to `struct cui_node`.

**Rationale**: The standard pattern for O(1) tail-append in an intrusive singly-linked child list is to cache a `last_child` pointer on the parent. This avoids the O(n) walk in `cui_node_append_child` while keeping the existing `first_child`/`next_sibling` iteration pattern intact.

**Alternatives considered**:
- **Doubly-linked sibling list (`prev_sibling`)**: Enables O(1) removal too, but adds a pointer per node and requires updating `prev_sibling` on detach/reorder. Over-engineering for the current use case where children are only appended, never removed mid-frame (arena reset clears everything).
- **Separate tail pointer external to `cui_node`**: Would require passing context around or storing per-parent tail in a separate structure. Adds indirection and complexity with no benefit over an inline field.
- **Circular sibling list**: `first_child->prev = last_child`. Clever but violates Constitution Principle I (Clarity Over Cleverness) and complicates every traversal loop.

## R2: Impact analysis on existing traversals

**Decision**: All existing traversals are read-only on `first_child`/`next_sibling`; none need modification.

**Rationale**: Audited every file that uses `first_child` or `next_sibling`:

| File | Pattern | Reads `last_child`? |
|------|---------|---------------------|
| `src/layout/layout.c` | `for (c = n->first_child; c; c = c->next_sibling)` | No |
| `src/core/draw_cmd.c` | Same iteration pattern | No |
| `src/core/a11y.c` | Same iteration pattern | No |
| `src/core/context.c` | Same iteration pattern | No |
| `src/core/diff.c` | Stub — no tree walk | No |
| `src/widget/*.c` | Call `cui_node_append_child` only | No (callers) |
| `tests/unit/test_layout.c` | Calls `cui_node_append_child` | No (caller) |

No traversal reads or writes `last_child`, so the field is purely maintained by the append function.

## R3: Memory/struct-size impact

**Decision**: Accept the +8 bytes (64-bit) / +4 bytes (32-bit) per `cui_node`.

**Rationale**: `sizeof(cui_node)` is currently dominated by `cui_node_layout` (10 floats + 2 ints = 48 bytes) plus other fields. Adding one pointer grows the struct by ~2–4%. On a 1000-node tree, total overhead is ~8KB — negligible compared to the 4MB default arena.

## R4: Initialization guarantee

**Decision**: Rely on existing `memset(n, 0, sizeof(cui_node))` in `cui_node_alloc`.

**Rationale**: The zero-init already covers `first_child` and `next_sibling` — `last_child` at the same struct level will also be zeroed. No additional initialization code needed.
