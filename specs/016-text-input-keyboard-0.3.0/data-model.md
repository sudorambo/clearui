# Data Model: Text Input & Keyboard (0.3.0)

## Retained node (cui_node) — text input fields

| Field (existing)   | Type    | Description |
|--------------------|---------|-------------|
| text_input_buf     | char *  | Caller-owned buffer; content modified by library on insert/delete. |
| text_input_cap     | size_t  | Capacity; insert must not write beyond capacity-1 (NUL). |

| Field (new)        | Type    | Description |
|--------------------|---------|-------------|
| text_input_cursor  | int or size_t | Cursor index in [0, strlen(buffer)]. Persisted via diff (copy declared↔retained). |

Cursor is the insertion point: insert places the new character at cursor and increments cursor; backspace removes the character before cursor and decrements cursor; delete removes the character at cursor.

## Context (cui_ctx) — per-frame / persistent

| Field (existing) | Description |
|------------------|-------------|
| pending_key      | Consumed in process_pending_key; Tab/Enter/Space/Backspace/Delete. |
| focusable_ids, focused_index | Identifies which widget is focused; when it’s a text input, key and char go there. |

| Field (new)       | Type     | Description |
|-------------------|----------|-------------|
| pending_char      | unsigned int | Pending printable character (codepoint); 0 = none. Consumed when processing focused text input (in end_frame or when applying input). |
| text_input_changed_id | char[CUI_LAST_CLICKED_ID_MAX] or similar | Id of the text input whose buffer was modified this frame. Cleared at begin_frame. When cui_text_input(ctx, id, ...) is called, return 1 if id matches this (and optionally clear). |

## Buffer and cursor invariants

- Buffer is always NUL-terminated after any library edit.
- length = strlen(buffer) ≤ capacity - 1 (after insert we never exceed).
- Cursor in [0, length]; after insert cursor = old_cursor + 1; after backspace cursor = max(0, old_cursor - 1); after delete cursor unchanged (content shifts).

## State transitions

1. **Focus**: Click or Tab sets focused_index to the text input; that widget’s retained node holds the cursor.
2. **Insert**: App/platform calls cui_inject_char(ctx, codepoint). In end_frame, if focused is text input and codepoint is printable, insert at retained node’s cursor into buffer, advance cursor, set text_input_changed_id.
3. **Backspace/Delete**: process_pending_key sees key and focused text input; mutate buffer and cursor from retained node; set text_input_changed_id.
4. **Return value**: Next frame, cui_text_input(ctx, id, ...) compares id to text_input_changed_id; if match return 1 else 0; clear or leave cleared at begin_frame.
