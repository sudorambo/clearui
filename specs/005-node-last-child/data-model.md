# Data Model: `cui_node` — `last_child` addition

## Entity: `cui_node` (modified)

**Location**: `src/core/node.h`

### Fields (change only)

| Field | Type | Description | New? |
|-------|------|-------------|------|
| `first_child` | `cui_node *` | Pointer to first child in sibling chain | Existing |
| `last_child` | `cui_node *` | Pointer to last child in sibling chain (tail cache) | **New** |
| `next_sibling` | `cui_node *` | Pointer to next sibling | Existing |

### Invariants

1. If `parent->first_child == NULL`, then `parent->last_child == NULL`.
2. If `parent->first_child != NULL`, then `parent->last_child` points to the last node in the `next_sibling` chain starting from `first_child`.
3. `parent->last_child->next_sibling == NULL` (the tail has no next sibling).
4. After `cui_node_alloc`, `last_child == NULL` (zero-initialized by `memset`).

### State Transitions

| Operation | `first_child` | `last_child` | Notes |
|-----------|---------------|--------------|-------|
| `cui_node_alloc` | `NULL` | `NULL` | Zero-init |
| Append 1st child | → child | → child | Both point to same node |
| Append Nth child | unchanged | → new child | `old_last->next_sibling = child` |
| `cui_arena_reset` | invalidated | invalidated | Arena wipes all nodes |

## Relationships

- `cui_node` → `cui_node` (parent → children via `first_child`/`last_child`/`next_sibling`)
- No new relationships introduced. `last_child` is a redundant cache of data already derivable from the `first_child`/`next_sibling` chain.
