# Data Model: Diff Engine

## Entities

### Declared tree (input)

- **Source**: Built by the app each frame via widget APIs; root in `ctx->declared_root`.
- **Storage**: Frame arena. All nodes allocated from `ctx->arena`; **never freed by the app** (arena reset at next `cui_begin_frame`).
- **Lifetime**: Valid only until next `cui_begin_frame`.

### Retained tree (output, persistent)

- **Source**: Output of `cui_diff_run(declared, &retained)`. Structure mirrors declared; nodes are **reused** when match is found, **allocated** when new, **freed** when no longer in declared.
- **Storage**: Context-owned. `ctx->retained_root` points to the root. Retained nodes are heap-allocated (e.g. `malloc` in diff) or from a dedicated retained arena.
- **Lifetime**: From first diff that produces it until (a) node is orphaned (freed by diff), or (b) context destroy (entire tree freed by context).

### Matching

- **Rule**: For each declared node, find the corresponding retained node:
  - **By key** (if present): e.g. `button_id` for BUTTON/CHECKBOX/TEXT_INPUT; match retained child with same key within same parent.
  - **By index**: If no key or no key match, match by child index (i-th declared child ↔ i-th retained child).
- **Reuse**: When a retained node is matched, reuse it (do not allocate a new one). Update its type/children to match declared; copy persistent state from retained **into** the declared node (so layout/draw see it).

### State copy (retained → declared)

| Node type   | Fields copied from retained to declared |
|------------|-----------------------------------------|
| CUI_NODE_SCROLL      | `scroll_offset_y` |
| CUI_NODE_TEXT_INPUT  | `text_input_buf`, `text_input_cap` (and ensure buffer is still valid or re-bound) |
| CUI_NODE_CHECKBOX    | `checkbox_checked` (pointer) |
| Others               | None for "start" |

### Lifecycle (context)

- **Create**: `retained_root = NULL`.
- **Begin frame**: Declared tree is rebuilt (arena reset); retained_root is unchanged.
- **End frame**: `cui_diff_run(declared_root, &retained_root)`; after this, retained_root mirrors declared and declared nodes have state copied from retained where matched.
- **Destroy**: Free entire retained tree (recursive free children then root); set `retained_root = NULL`.

## Relationships

- One declared tree per frame (arena).
- One retained tree per context (persists across frames).
- Diff is the only writer of retained_root; context passes it in and owns the pointer.
