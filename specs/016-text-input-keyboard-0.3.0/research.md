# Research: Text Input & Keyboard (0.3.0)

Decisions for character injection API, cursor storage, backspace/delete handling, and “changed” return value.

---

## 1. Character injection API

**Decision**: Add `void cui_inject_char(cui_ctx *ctx, unsigned int codepoint)`. Document that typical usage is ASCII (codepoint < 128); non-printable/control characters are ignored. Platform or app converts platform key events to codepoints and calls this (e.g. on keydown for printable key).

**Rationale**: Single explicit API for “this character was typed”; keeps key handling (Tab, Enter, Backspace) separate from character input. `unsigned int` allows future UTF-8 codepoints without API change; for 0.3.0 we only insert when codepoint is printable (e.g. 32–126).

**Alternatives considered**: (a) Reuse `cui_inject_key` with key codes for each printable — too many codes and platform-dependent. (b) `cui_inject_char(ctx, char c)` — too narrow for non-ASCII later. (c) Queue of characters — unnecessary for single-threaded, one-event-per-frame; one pending char per frame is enough.

---

## 2. Cursor position storage

**Decision**: Store cursor index in the retained node: add `int text_input_cursor` (or `size_t`) to `cui_node`. Diff copies it declared←retained and retained←declared so it persists across frames. When the focused widget is a text input, insert/delete/backspace use the retained node’s cursor for that widget; after diff, the declared node gets the cursor from retained.

**Rationale**: Matches existing pattern (e.g. `scroll_offset_y` on scroll nodes). One cursor per text input instance; no extra keyed storage in context. Cursor is index in [0, len] where len = strlen(buffer).

**Alternatives considered**: Context map keyed by widget id — works but duplicates “state per widget” that the retained tree already represents. Retained node is the single source of truth.

---

## 3. Backspace and Delete

**Decision**: Add `CUI_KEY_BACKSPACE` and `CUI_KEY_DELETE` (e.g. 0x0104, 0x0105). In `process_pending_key`, when the focused widget is a text input (identified by focusable_ids[focused_index]), run backspace or delete on that widget’s buffer and cursor (using retained node’s cursor, or the node we find by id). Do not synthesize a click for text input on Enter/Space if we want “commit” behavior later; for 0.3.0 Enter/Space can still activate (e.g. submit). Backspace: if cursor > 0, remove char at cursor-1, decrement cursor. Delete: if cursor < len, remove char at cursor.

**Rationale**: Reuses the same key pipeline as Tab/Enter/Space; no second injection API. Implementation: in process_pending_key, if key is backspace/delete and focused_id corresponds to a text input, resolve retained node (or buffer) for that id and apply edit.

**Alternatives considered**: Separate `cui_inject_backspace` — redundant with key codes. Handling only in widget layer — key processing is in context; context has focus and can dispatch to “focused text input” logic.

---

## 4. Return 1 when buffer content changed

**Decision**: When we mutate a text input’s buffer (insert or backspace/delete), set a “changed id” in context (e.g. `ctx->text_input_changed_id` or a flag per id). During declaration, `cui_text_input` is called with an id; after end_frame we have already run process_pending_key and possibly applied inject_char. So “changed” applies to the previous frame. Store in context the id of the text input that was modified this frame (in process_pending_key / character application). When `cui_text_input(ctx, id, buffer, cap, opts)` is called in the next frame, compare id to stored “last changed id”; if equal, return 1 and clear the stored id (or clear at begin_frame). So: in end_frame we set changed_id when we edit; in the next frame’s declaration, cui_text_input returns 1 if its id == changed_id.

**Rationale**: Caller sees “this widget’s buffer was changed last frame” without polling; one-frame latency is acceptable for “content changed” feedback.

**Alternatives considered**: Compare buffer before/after in cui_text_input — expensive and fragile. Dirty flag on the node — retained node is internal; the public API is cui_text_input return value, so we need to map “which widget changed” to the declaration of that widget next frame; storing the id in context is the minimal way.

---

## 5. Buffer full and cursor clamping

**Decision**: Before insert, if strlen(buffer) >= capacity - 1, do not insert (buffer is full). After any edit or when loading retained cursor, clamp cursor to [0, strlen(buffer)]. When buffer is externally changed by the app, we don’t track it; clamping on next use keeps cursor valid.

**Rationale**: Prevents overrun; keeps cursor in range without complex invariants.
