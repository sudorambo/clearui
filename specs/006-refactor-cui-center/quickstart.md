# Quickstart: Verifying `cui_center` Refactor

## Build & Test

```bash
make clean && make all && make unit-tests && make integration-tests
```

All tests pass with zero warnings. No API changes — existing code works identically.

## What Changed

`cui_center` in `src/widget/layout.c` was reduced from 15 lines of manually duplicated container-setup code to a 2-line delegation:

Before:
```c
void cui_center(cui_ctx *ctx) {
    if (!ctx) return;
    cui_node **root = cui_ctx_root_ptr(ctx);
    cui_node *center = cui_node_alloc(cui_ctx_arena(ctx));
    if (!center) return;
    center->type = CUI_NODE_CENTER;
    center->layout_w = 400;
    center->layout_h = 300;
    if (!*root) *root = center;
    else {
        cui_node *parent = cui_ctx_current_parent(ctx);
        if (parent) cui_node_append_child(parent, center);
        else *root = center;
    }
    cui_ctx_push_parent(ctx, center);
}
```

After:
```c
void cui_center(cui_ctx *ctx) {
    if (!ctx) return;
    push_container(ctx, CUI_NODE_CENTER, NULL, 400, 300);
}
```

Now consistent with `cui_row`, `cui_column`, `cui_stack`, and `cui_wrap`.

## Verification

- All 3 integration tests (`test_hello`, `test_counter`, `test_rdi_platform`) use `cui_center` and pass.
- `test_layout` unit test passes (layout engine unchanged).
- `cui_center` no longer duplicates `push_container` logic — `grep` confirms only `push_container` calls `cui_ctx_root_ptr`, `cui_node_alloc`, and `cui_ctx_push_parent` in `layout.c`.
