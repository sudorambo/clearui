# Data Model: Robustness & Limits (0.9.0)

## Entities

### Error callback (optional)

- **Role**: Application-provided function called when a limit is hit or misuse is detected (when set).
- **Signature** (conceptual): `void (*cui_error_fn)(void *userdata, int error_code, const char *limit_name_or_null);`
- **Error codes** (conceptual): e.g. `CUI_ERR_PARENT_STACK`, `CUI_ERR_FOCUSABLE_FULL`, `CUI_ERR_A11Y_FULL`, `CUI_ERR_ID_TRUNCATED`, `CUI_ERR_UNBALANCED` (for CUI_DEBUG).
- **Storage**: Function pointer and userdata in `cui_config` or `cui_ctx`; set at create or via setter. NULL = no callback (current silent behavior).

### Limits (existing, behavior extended)

- **Parent stack**: On push when `parent_top >= CUI_PARENT_STACK_MAX`, do not push; call error callback if set; in CUI_DEBUG assert.
- **Focusable list**: When full, skip adding; call error callback if set; optionally warn.
- **A11y list**: When full, skip adding; call error callback if set.
- **Widget ID truncation**: When copying ID into fixed buffer, if truncated, call error callback if set; in CUI_DEBUG assert or warn.

### UTF-8 decoder (utf8_next or shared helper)

- **Input**: Buffer (NUL-terminated or length-bound per contract).
- **Output**: Next codepoint and byte advance; or 0 advance on error/malformed.
- **Validation**: Reject overlong sequences; reject surrogates (U+D800–U+DFFF); do not read past end of buffer (length or NUL).

### Fuzz / stress / leak

- **Fuzz target**: Standalone binary or harness that takes bytes and calls decoder (and optionally vault, frame allocator); no persistent UI state.
- **Stress test**: Program that builds large tree (1000+ nodes or deep nesting), runs N frames; success = no crash.
- **Leak CI**: Job that runs tests under Valgrind or with LeakSanitizer; success = no leaks reported.

## State transitions

- **Set error callback**: Once set at create or via setter, all subsequent limit hits (and optionally CUI_DEBUG misuse) invoke it until cleared or context destroyed.
- **utf8_next**: No persistent state; each call is stateless given the current pointer.

## Validation rules

- Callback must be callable from within library (no re-entrant API calls that could trigger callback again in same frame if not careful; document or avoid).
- UTF-8 helper must not read more than length or past NUL.
