# Contract: Text Input & Keyboard API (0.3.0)

## New API

### cui_inject_char

```c
void cui_inject_char(cui_ctx *ctx, unsigned int codepoint);
```

- **Semantics**: Injects one printable character for the currently focused text input. If the focused widget is not a text input, the character is ignored. If codepoint is not printable (e.g. control character), it is ignored. Typical use: codepoint 32–126 (ASCII printable). Platform or application converts platform key events to codepoints and calls this (e.g. on keydown for character key).
- **When applied**: Processed during end_frame (or when the library applies input), same as pending_key. Only one character per frame if we use a single pending_char slot.
- **Insertion**: Character is inserted at the current cursor position; cursor advances by one. Buffer remains NUL-terminated; if buffer is full (strlen(buffer) >= capacity - 1), insert is a no-op.

### New key codes

```c
#define CUI_KEY_BACKSPACE  0x0104
#define CUI_KEY_DELETE     0x0105
```

- **CUI_KEY_BACKSPACE**: When the focused widget is a text input, delete the character before the cursor and move cursor back by one. If cursor is 0, no-op.
- **CUI_KEY_DELETE**: When the focused widget is a text input, delete the character at the cursor (content shifts left). If cursor is at end of buffer, no-op.

### cui_text_input return value

```c
int cui_text_input(cui_ctx *ctx, const char *id, char *buffer, size_t capacity, const cui_text_input_opts *opts);
```

- **Return 1**: The buffer for this text input (same id) was modified in the previous frame (character inserted or backspace/delete applied).
- **Return 0**: No modification last frame, or this is the first frame, or id does not match the widget that was edited.

Caller can use return value 1 to trigger save, validation, or other “content changed” logic.

## Existing API (unchanged)

- `cui_inject_key(ctx, CUI_KEY_TAB)` / `CUI_KEY_SHIFT_TAB`: Focus navigation.
- `cui_inject_key(ctx, CUI_KEY_ENTER)` / `CUI_KEY_SPACE`: Activate focused widget (e.g. button); for text input, behavior can remain “activate” or do nothing; 0.3.0 does not require Enter to “submit” the field.
- Buffer and capacity are caller-owned; the library only writes into the buffer on insert/delete.
