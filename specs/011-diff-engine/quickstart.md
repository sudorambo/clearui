# Quickstart: Diff Engine

## What the diff engine does

Each frame the app builds a **declared** UI tree (e.g. `cui_center` → `cui_label` → …). The library keeps a **retained** tree from the previous frame. The diff engine:

1. **Reconciles** structure: retained tree is updated to mirror the declared tree (same types, same child order).
2. **Matches** nodes: same logical widget (by key like `button_id` or by child index) reuses the retained node.
3. **Copies state** from retained into declared for matched nodes (scroll position, text input content, checkbox pointer) so layout and draw see the correct state.
4. **Frees** retained nodes that are no longer in the declared tree.

So: focus, scroll, and text input persist across frames without the app storing that state itself (or in addition to Vault for app-level state).

## For implementers

1. **Retained allocation**: In `diff.c`, when a declared node has no matching retained node, allocate a new node (e.g. `malloc(sizeof(cui_node))`), init from declared (type, layout_opts, button_id, etc.), and attach to the new retained tree. When a retained node is not matched to any declared node, free its subtree and itself.
2. **Matching**: Walk declared and retained in parallel (same parent). For each declared child, try key match first (e.g. `button_id`), else match by index (i-th child). If matched, reuse retained node and copy persistent state from retained into the declared node. If no match, create new retained node and copy from declared.
3. **State copy**: After matching, for each matched pair (declared, retained), copy `scroll_offset_y`, `text_input_buf`/`text_input_cap`, `checkbox_checked` from retained into declared (so layout/draw use declared tree with correct state).
4. **Context destroy**: In `cui_destroy`, free the entire retained tree (e.g. recursive helper that frees children then node).

## Verification

- Two frames with identical declaration: retained tree after second frame has same structure; scroll or text input state preserved.
- Remove a widget: corresponding retained subtree is freed (no leak).
- `make clean && make all && make unit-tests && make integration-tests` passes.
