# Research: Start the Diff Engine

## Decision 1: Matching strategy (how to pair declared vs retained nodes)

**Decision**: Match nodes by **child index** in the first iteration: the i-th child of a declared parent matches the i-th child of the corresponding retained parent. Where a stable **key** exists (e.g. `button_id` for buttons, or a dedicated `diff_key` field), use it to match within the same parent so that reordering or insertions don't wrongfully reuse a retained node. If no key, index-only matching is acceptable for "start" and preserves state for stable declaration order.

**Rationale**: Index-only is simplest and works for many UIs; key-based matching improves correctness when widgets are reordered or conditionally inserted. Starting with index keeps scope small; we can add key matching when `button_id` (or similar) is present.

**Alternatives considered**:
- **Key-only**: Would require every node to have a key; many nodes (spacers, labels) don't have a natural key. Rejected for first iteration.
- **Deep equality of type + children**: Too expensive and doesn't identify "same" widget across frames. Rejected.

---

## Decision 2: Where retained nodes live and who frees them

**Decision**: Retained nodes are **heap-allocated** (malloc per node, or a small pool). The **context** owns the retained root pointer; in `cui_destroy`, the context frees the entire retained tree (recursive free of children then self). During diff, when a retained node is no longer needed (not matched to any declared node), the diff implementation frees that retained subtree. Allocation for new retained nodes: malloc in diff, or a context-owned "retained arena" that is only cleared when the context is destroyed—arena avoids per-node malloc/free at the cost of not reclaiming memory until destroy. For "start", **malloc/free per node** is acceptable and makes ownership clear.

**Rationale**: Constitution requires explicit ownership; context owns retained_root and is responsible for tearing it down. Per-node malloc in diff is visible and debuggable; a retained arena can be a follow-up optimization.

**Alternatives considered**:
- **Retained arena (reset never)**: Would require copying declared content into retained nodes allocated from that arena; then we don't free individual nodes, only reset on destroy. Simpler lifecycle but less granular. Can be added later.
- **Store retained state in a separate structure (e.g. hash by key)** instead of a full tree: Would diverge from "retained tree mirrors declared"; layout/draw currently expect a tree. Rejected for this feature.

---

## Decision 3: State transfer direction (where does layout/draw read state?)

**Decision**: **Copy state from retained into declared** before layout. After diff runs, the diff updates the retained tree to mirror declared structure and copies persistent state (e.g. `scroll_offset_y`, `text_input_buf`, `checkbox_checked` pointer) from retained nodes into the corresponding declared nodes. Layout and draw then run on the **declared** tree (as today), so they see the updated state. Declared tree remains the single source of truth for structure; retained is the cache of state.

**Rationale**: Minimal change to layout and draw—they already run on declared_root. No need to run layout on retained; we just inject state into declared so that the rest of the pipeline works unchanged.

**Alternatives considered**:
- **Run layout on retained tree**: Would require layout to mutate retained nodes and draw to read from retained; larger change and duplicates logic. Rejected.
- **Keep state only in retained, layout reads both**: More complex and error-prone. Rejected.

---

## Decision 4: What state is "persistent" and must be copied

**Decision**: For the first iteration, copy at least: **scroll_offset_y** (SCROLL), **text_input_buf** and **text_input_cap** (TEXT_INPUT), **checkbox_checked** pointer (CHECKBOX). Optionally **focus** is implied by which node has focus (handled by context/focusable list, not necessarily in the node). Copy only for nodes that were **matched** (same position or same key).

**Rationale**: These are the stateful widgets that users expect to persist across frames. Others (label text, button_id) are typically set by the app each frame from declaration.

**Alternatives considered**:
- **Copy everything from retained to declared**: Could overwrite app-set values (e.g. label_text). Rejected; only copy designated persistent fields.
- **Copy nothing, only structural reconcile**: Would not preserve scroll or input. Rejected.
