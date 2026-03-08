# Migration Guide

Upgrade notes for ClearUI from 0.1.0 through 1.0.0. See [CHANGELOG.md](../CHANGELOG.md) for full release details.

## 1.0.0

- **Stable release**: First stable version. All milestones complete; public API is frozen (no breaking changes until 2.0).
- **Version**: `cui_version_string()` now returns `"1.0.0"`.
- No API changes from 0.10.0.

## 0.10.0

- **Added**: `cui_version_string(void)` — returns runtime version string (e.g. `"0.10.0"`). Use for logging or about dialogs.
- **Clarified**: `cui_layout` padding: `padding_x` / `padding_y` override `padding` when &gt; 0; otherwise `padding` applies to both axes. No API change; behavior unchanged.
- **API freeze**: After 0.10.0 there are no breaking public API changes until 1.0. New APIs will be additive only.

## 0.9.0

- **Added**: Optional `cui_config.error_callback` and `error_userdata`; define `CUI_DEBUG` for development assertions. UTF-8 robustness, fuzz targets, stress test, leak-check targets. No breaking changes.

## 0.8.0

- **Added**: Software RDI rasterization, viewport, present_software, font glyph APIs. All additive.

## 0.7.0

- **Breaking**: `poll_events(platform_ctx)` → `poll_events(platform_ctx, void *cui_ctx)`. Update your platform adapter to accept the second argument (ClearUI context for event injection); pass it to `cui_inject_*` calls as needed.

## 0.6.0

- **Breaking**: `cui_layout_run(root, viewport_w, viewport_h)` → `cui_layout_run(ctx, root, viewport_w, viewport_h)`. Add the context as the first argument. Internal layout API; most apps use only the widget layer.
- **Added**: `cui_set_theme`, `cui_theme_dark`, theme struct. Pass `NULL` to `cui_set_theme` to reset to default.

## 0.5.0

- No public API changes. Bug fix: canvas draw commands now replay correctly.

## 0.4.0

- **Breaking**: `cui_scroll(ctx, opts)` → `cui_scroll(ctx, id, opts)`. Add an `id` (const char *); use `NULL` if this scroll region should not receive scroll wheel events. Scroll containers with a non-NULL id under the pointer receive wheel input.

## 0.3.0

- **Added**: `cui_inject_char`, backspace/delete keys, `cui_text_input` return value (1 when buffer changed). No breaking signature changes.

## 0.2.0

- **Added**: CHANGELOG, docs for color format and limits. No breaking API changes. CI and sanitizer targets added.

## Upgrading from 0.1.0

1. **0.4.0**: Update every `cui_scroll(ctx, &opts)` to `cui_scroll(ctx, NULL, &opts)` or pass an id string.
2. **0.6.0**: If you call `cui_layout_run` directly, add `ctx` as the first argument.
3. **0.7.0**: If you implement `cui_platform`, update `poll_events` to take two arguments and use the second (cui_ctx *) for event injection.
