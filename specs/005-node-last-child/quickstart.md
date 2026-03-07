# Quickstart: Verifying `last_child` O(1) Appends

## Build & Test

```bash
make clean && make all && make unit-tests && make integration-tests
```

All tests pass with zero warnings. No API changes — existing code works identically.

## What Changed

`cui_node_append_child(parent, child)` now runs in O(1) instead of O(n).

Before (internal — walks sibling chain):
```c
cui_node *p = parent->first_child;
while (p->next_sibling) p = p->next_sibling;
p->next_sibling = child;
```

After (internal — uses cached tail pointer):
```c
parent->last_child->next_sibling = child;
parent->last_child = child;
```

No public API or behavioral change. All widgets, layout, a11y, and rendering work exactly as before.

## Verification

The `test_layout` binary includes a new test section that appends 3 children and verifies:
- `parent->first_child` points to the first appended child
- `parent->last_child` points to the most recently appended child
- The sibling chain is intact (`a→b→c`, `c->next_sibling == NULL`)
