# Contract: Public API (clearui.h)

ClearUI’s public API is the set of functions and types exposed in `clearui.h` and used by application code. All public symbols are prefixed with `cui_` (functions) or `CUI_` (macros/constants).

## Invariants

- **Context**: Every function that takes `cui_ctx *ctx` requires a non-null context created by `cui_create()` and not yet destroyed. Use after `cui_destroy(ctx)` is undefined.
- **Frame**: All UI declaration and frame allocation (e.g. `cui_frame_printf`) MUST occur between `cui_begin_frame(ctx)` and `cui_end_frame(ctx)`. Paired begin/end required.
- **Threading**: A single `cui_ctx` MUST be used from one thread only (typically the UI thread).
- **Naming**: Public API surface MUST NOT exceed ~120 functions; target 60–80 for a complete app.

## Categories

1. **Lifecycle**: `cui_create`, `cui_destroy`, `cui_running`, `cui_begin_frame`, `cui_end_frame`, optional `cui_run`.
2. **Layout containers**: `cui_row`, `cui_column`, `cui_stack`, `cui_center`, `cui_wrap`, `cui_scroll`, each with `cui_end()` to close.
3. **Widgets**: `cui_button`, `cui_label`, `cui_label_styled`, `cui_text_input`, `cui_checkbox`, `cui_icon_button`, `cui_spacer`, `cui_canvas`, etc.
4. **State**: `cui_state(ctx, key, size)` → pointer to persistent blob; `cui_push_style` / `cui_pop_style`.
5. **Frame memory**: `cui_frame_alloc`, `cui_frame_printf` (valid until next `cui_begin_frame`).
6. **Configuration**: `cui_config` struct for `cui_create`; layout/options structs (e.g. `cui_layout`, `cui_input_opts`) with designated initializers.
7. **Accessibility**: `cui_aria_label`, `cui_tab_index` (optional overrides).
8. **Canvas / custom draw**: `cui_canvas`, `cui_canvas_draw_list`, `cui_draw_rect`, `cui_draw_circle`, `cui_draw_text`, etc.

## Out of scope (not guaranteed in this contract)

- Internal types (node descriptors, arena, vault internals).
- RDI or platform function pointers (applications may set or ignore them depending on build).

## Versioning

Breaking changes to the public API (removals, signature changes, semantic changes) require a MAJOR version bump and a migration path documented in the release notes.
