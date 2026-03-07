# Research: Refactor `cui_center` to use `push_container`

## R1: Is `push_container` semantically equivalent to `cui_center`'s manual code?

**Decision**: Yes — the two code paths are identical.

**Rationale**: Line-by-line comparison of `cui_center` (lines 39–54) vs `push_container(ctx, CUI_NODE_CENTER, NULL, 400, 300)`:

| Step | `cui_center` (manual) | `push_container` (helper) |
|------|----------------------|--------------------------|
| Get root pointer | `cui_ctx_root_ptr(ctx)` | `cui_ctx_root_ptr(ctx)` |
| Allocate node | `cui_node_alloc(cui_ctx_arena(ctx))` | `cui_node_alloc(cui_ctx_arena(ctx))` |
| Set type | `center->type = CUI_NODE_CENTER` | `n->type = type` (CUI_NODE_CENTER) |
| Set default dims | `center->layout_w = 400; layout_h = 300` | `n->layout_w = default_w; n->layout_h = default_h` (400, 300) |
| Copy layout opts | (none — no opts) | `copy_layout_opts(&n->layout_opts, NULL)` → `memset` to zero, early return |
| Attach to tree | if/else root/parent logic | Identical if/else root/parent logic |
| Push parent | `cui_ctx_push_parent(ctx, center)` | `cui_ctx_push_parent(ctx, n)` |

The only "extra" work in `push_container` is `copy_layout_opts(dst, NULL)`, which does `memset(dst, 0, sizeof(*dst))` then returns immediately because `src == NULL`. Since `cui_node_alloc` already `memset`s the entire node to zero (including `layout_opts`), this is a redundant but harmless no-op.

**Alternatives considered**:
- **Inline `push_container` into all callers**: Would eliminate the function entirely but increase code size and duplication. Worse.
- **Add `opts` parameter to `cui_center`**: Out of scope per spec. Could be a follow-up feature.

## R2: Does passing `NULL` as `opts` cause any issue?

**Decision**: No. `copy_layout_opts` handles `NULL` explicitly with an early return after zeroing the destination.

**Rationale**: From `layout.c` line 6–19:
```c
static void copy_layout_opts(cui_node_layout *dst, const cui_layout *src) {
    memset(dst, 0, sizeof(*dst));
    if (!src) return;  // <-- NULL handled here
    ...
}
```

This is safe and already tested by the fact that the node is zero-initialized by `cui_node_alloc` anyway.
