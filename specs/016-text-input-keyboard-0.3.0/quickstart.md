# Quickstart: Text Input & Keyboard (0.3.0)

## Running tests

```bash
make clean && make all && make unit-tests && make integration-tests
```

New tests:

- **Unit**: `test_text_input` — insert character, backspace, delete, cursor clamping, buffer full.
- **Integration**: `test_text_input_edit` — focus text input, inject characters (and optionally backspace/delete), run frame, assert buffer content.

## Minimal usage (after 0.3.0)

```c
char buf[64] = {0};
cui_text_input(ctx, "name", buf, sizeof(buf), &(cui_text_input_opts){ .placeholder = "Name" });

// In platform event handler (or test):
cui_inject_click(ctx, x, y);   // focus the field (if hit-test selects it)
cui_inject_char(ctx, 'a');
cui_inject_char(ctx, 'b');
cui_inject_key(ctx, CUI_KEY_BACKSPACE);  // now buf is "a"
cui_end_frame(ctx);

// Next frame:
int changed = cui_text_input(ctx, "name", buf, sizeof(buf), &opts);
// changed == 1 if we injected char or backspace/delete last frame
```

## Where things live

| Item            | Location |
|-----------------|----------|
| Public API      | `include/clearui.h` — `cui_inject_char`, `CUI_KEY_BACKSPACE`, `CUI_KEY_DELETE`, `cui_text_input` return value. |
| Cursor storage  | `src/core/node.h` — `text_input_cursor` on node; `src/core/diff.c` — copy cursor in diff. |
| Key/char handling | `src/core/context.c` — `process_pending_key`, pending_char, text_input_changed_id. |
| Widget          | `src/widget/text_input.c` — register focus, return 1 when id matches changed_id. |
| Unit test       | `tests/unit/test_text_input.c` |
| Integration test | `tests/integration/test_text_input_edit.c` |
