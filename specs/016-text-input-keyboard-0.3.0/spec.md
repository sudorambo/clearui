# Feature Specification: Text Input & Keyboard (0.3.0)

**Feature Branch**: `016-text-input-keyboard-0.3.0`  
**Created**: 2026-03-07  
**Status**: Draft  
**Input**: ROADMAP.md Milestone 2 — complete `cui_text_input` so users can type into it.

## User Scenarios & Testing

### User Story 1 - User types characters into a focused text field (Priority: P1)

As a user I can focus a text input (click or Tab), then type printable characters so that they appear in the buffer and I see the result.

**Why this priority**: Core expectation of a text field.

**Independent Test**: Focus the field, call `cui_inject_char` (or equivalent) for a sequence of characters; read back the buffer and assert content matches.

**Acceptance Scenarios**:

1. **Given** a focused text input with empty buffer, **When** the app injects characters "abc", **Then** the buffer contains "abc" and the widget displays it.
2. **Given** a focused text input with "ab", **When** the app injects "c", **Then** the buffer becomes "abc" (insertion at cursor).

---

### User Story 2 - User moves cursor and edits with backspace/delete (Priority: P1)

As a user I can move the cursor (e.g. arrow keys or click—click can be later), and use backspace/delete to remove characters so that editing feels natural.

**Why this priority**: Without cursor and backspace/delete, the field is not usable for editing.

**Independent Test**: Set buffer to "hello", set cursor to after "l", inject backspace; buffer becomes "helo". Inject delete at start; character after cursor is removed.

**Acceptance Scenarios**:

1. **Given** buffer "hello" and cursor at 5, **When** backspace is injected, **Then** buffer becomes "hell" and cursor 4.
2. **Given** buffer "hello" and cursor at 0, **When** delete is injected, **Then** buffer becomes "ello" and cursor stays 0.

---

### User Story 3 - App knows when the buffer changed (Priority: P2)

As an application I can call `cui_text_input` and get return value 1 when the buffer content changed this frame so that I can save or validate.

**Why this priority**: Enables "save on change" and form validation.

**Independent Test**: Call `cui_text_input`, inject a character, call `cui_end_frame` and re-run the frame; on the next `cui_text_input` call (or end of frame) the function returns 1 for that widget.

**Acceptance Scenarios**:

1. **Given** no user input this frame, **When** `cui_text_input` returns, **Then** it returns 0.
2. **Given** user typed or deleted this frame, **When** `cui_text_input` returns (or a follow-up query), **Then** it returns 1 for that id.

---

### User Story 4 - Tests cover text input editing (Priority: P2)

As a maintainer I have unit tests for character insertion, backspace, delete, and cursor, and an integration test that types into a field and reads back the buffer.

**Independent Test**: Run `make unit-tests` and `make integration-tests`; new tests pass.

---

### Edge Cases

- Buffer full: inserting a character when `strlen(buffer) >= capacity-1` should not overflow; optionally do nothing or truncate.
- Cursor out of range: after delete/backspace or external buffer change, clamp cursor to 0..len.
- Non-printable / control characters: only insert printable (or document that inject is for printable/codepoint); backspace/delete as special keys.
- Multiple text inputs: only the focused one receives `cui_inject_char` and backspace/delete.

## Requirements

### Functional Requirements

- **FR-001**: The library MUST provide a way to inject printable character input (e.g. `cui_inject_char(ctx, codepoint)` or `cui_inject_char(ctx, char)`). When the focused widget is a text input, the character is inserted at the cursor position.
- **FR-002**: The library MUST track cursor position per focused text input (retained across frames). Cursor is an index in the buffer (0 to length, inclusive).
- **FR-003**: The library MUST support backspace (delete character before cursor, cursor moves back) and delete (delete character at cursor). Either via new `CUI_KEY_BACKSPACE` / `CUI_KEY_DELETE` handled in existing key processing, or dedicated API.
- **FR-004**: `cui_text_input` MUST return 1 when the buffer content changed this frame (insert/delete), and 0 otherwise.
- **FR-005**: Unit tests MUST cover: insert character, backspace, delete, cursor clamping, buffer full.
- **FR-006**: An integration test MUST type into a text field (via inject API), run the frame loop, and assert the buffer content.

### Key Entities

- **Text input buffer**: Caller-owned `char *buffer`, `size_t capacity`; NUL-terminated. Content is modified by the library on insert/delete.
- **Cursor position**: Integer index in [0, strlen(buffer)]; retained per widget id (in retained node or context).
- **Pending character**: One pending printable character per frame (or queue), consumed when processing focused text input.

## Success Criteria

- **SC-001**: User can focus a text input and type characters; buffer and display update.
- **SC-002**: User can use backspace and delete to edit; cursor position is correct.
- **SC-003**: `cui_text_input` returns 1 when content changed, 0 otherwise.
- **SC-004**: New unit tests and one integration test pass in CI.
