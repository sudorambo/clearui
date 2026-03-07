# Data Model: ClearUI Core

**Feature**: 001-clearui-core  
**Date**: 2026-03-06

Entities and data structures for the ClearUI implementation. No persistent storage; all state is in-memory and tied to `cui_ctx` lifetime or frame lifetime.

---

## 1. Context and Global State

### `cui_ctx` (opaque)

- **Role**: Root handle for the library; holds all per-context state.
- **Ownership**: Created by `cui_create()`, destroyed by `cui_destroy()`. Single owner (application).
- **Contents (conceptual)**:
  - Arena allocator (e.g. 4 MB default, grow by double).
  - Frame bump allocator (reset each `cui_begin_frame`).
  - Vault (hash map: key → persistent blob).
  - Retained node tree (internal; diffed each frame).
  - Platform and RDI pointers (injected at create or set before first frame).
  - Config: window title, size, scale factor, debug flags.
- **Validation**: Non-null when passed to any `cui_*` API; use after destroy is undefined.

---

## 2. Frame and Allocation

### Arena (internal)

- **Role**: Backing store for UI node tree and layout data.
- **Lifetime**: From `cui_create()` to `cui_destroy()`. Logical reset at each `cui_begin_frame()` (pointer rewind).
- **Fields**: Base pointer, current offset, capacity. Grow by doubling when full; log diagnostic on grow.

### Frame allocator (internal)

- **Role**: Transient allocations (e.g. `cui_frame_printf`, temporary arrays).
- **Lifetime**: Valid until next `cui_begin_frame()`.
- **No explicit free**: Bump only; reset at start of frame.

### Vault entry (internal)

- **Key**: String (e.g. `"main.counter"`, `"todo.input"`). Hashed with FNV-1a.
- **Value**: Opaque blob of user-specified size; zero-initialized on first `cui_state()`.
- **Lifetime**: Until pruned (configurable N frames without access) or context destroy.
- **Thread safety**: Single-threaded; vault access only on UI thread.

---

## 3. Node Tree (Internal Retained + Declared)

### Node descriptor (internal)

- **Role**: Lightweight descriptor for one UI element produced by a declaration (e.g. `cui_button`, `cui_row`).
- **Fields (conceptual)**: Type (button, label, row, column, etc.), stable id/key, layout props (flex, gap, align), style, children count/link, optional payload (text, state pointer).
- **Storage**: Allocated from arena. No pointers into application heap for node payloads that outlive the frame; strings from `cui_frame_printf` are valid for the frame only.

### Declared tree (per frame)

- **Role**: Result of the user’s declaration calls during the frame (immediate-mode style).
- **Structure**: Built during the declaration phase; then diffed against retained tree. Diff result drives layout and draw.

### Retained tree (internal)

- **Role**: Previous frame’s tree (or empty on first frame). Compared with declared tree to compute minimal updates (animation, a11y, focus).

---

## 4. Layout

### `cui_layout` (public struct)

- **Role**: Layout options for container nodes (row, column, etc.).
- **Fields**: `gap`, `padding`, `padding_x`, `padding_y`, `max_width`, `min_width`, `max_height`, `flex`, `align`, `align_y`.
- **Defaults**: Zero or `CUI_ALIGN_START`; empty `(cui_layout){}` is valid.

### Layout result (internal)

- **Role**: Per-node computed position and size (logical pixels).
- **Fields**: `x`, `y`, `width`, `height`; optionally content box / scroll offset for scroll containers.
- **Computation**: Single pass over tree plus flex-grow pass; no persistent layout cache across frames (recomputed each frame from current tree).

---

## 5. Draw Commands

### Draw command buffer (internal)

- **Role**: Ordered list of draw operations produced by Core/Layout for the current frame.
- **Format**: Flat array of tagged unions (e.g. rect, rounded rect, line, text glyph, texture quad). No per-command heap allocation.
- **Consumer**: RDI implementation; may batch or sort by texture/state before GPU submit.

### Command types (conceptual)

- Clear (if needed).
- Filled rect (color).
- Rounded rect (color, radius).
- Line (start, end, color, thickness).
- Textured quad (uv, texture id).
- Text (glyph run, atlas, position).
- Scissor (rectangle).

---

## 6. Platform and RDI Interfaces

### `cui_platform` (public: struct of function pointers)

- **Role**: Abstraction over windowing, input, clipboard, cursor, GPU surface.
- **Operations**: create window, poll events, get clipboard, set cursor, create surface for RDI, etc.
- **Ownership**: Injected by application; ClearUI does not allocate the struct.

### `cui_rdi` or RDI struct (public: struct of function pointers)

- **Role**: Abstraction over GPU: init, texture create/upload, submit command buffer, scissor, present.
- **Ownership**: Injected or selected at compile time; one active driver per build.

---

## 7. Widget and Style State

### Style stack (internal)

- **Role**: Current style (colors, font size, etc.) applied to subsequent declarations. Push/pop (e.g. `cui_push_style`, `cui_pop_style`).
- **Storage**: Stack of style structs; per-frame; no persistent style tree.

### Focus and accessibility (internal)

- **Role**: Focus order derived from declaration order; overridable with `cui_tab_index`. A11y tree built from retained node tree: role, label, state, bounds.
- **Platform**: Mapped to UI Automation / NSAccessibility / AT-SPI2 in platform layer.

---

## 8. Key Entities Summary

| Entity           | Scope      | Lifetime           | Owner        |
|------------------|------------|--------------------|--------------|
| `cui_ctx`        | Per app    | create → destroy   | Application |
| Arena            | Per ctx    | ctx lifetime       | ClearUI      |
| Frame allocator  | Per frame  | begin_frame → next begin_frame | ClearUI |
| Vault            | Per ctx    | until prune/destroy| ClearUI      |
| Node tree        | Per frame  | frame (arena reset)| ClearUI      |
| Draw commands    | Per frame  | frame              | ClearUI      |
| `cui_layout`     | Per call   | value type         | Caller       |
| Platform / RDI   | Per ctx    | set at create/init | Application  |

---

## 9. Validation Rules

- **Context**: Every public API that takes `cui_ctx *` requires non-null context (documented; UB or assert in debug if null).
- **Frame discipline**: `cui_begin_frame` / `cui_end_frame` must be paired; no API that allocates from frame/arena outside a frame.
- **Vault keys**: Unique per context; same key in same frame returns same pointer. Key string must outlive the call (e.g. literal or static).
- **Layout**: Negative gap/padding can be clamped or undefined; implementation-defined. Flex ≥ 0.

No database or file schema; all state is transient (frame) or process-lifetime (context, vault until prune).
