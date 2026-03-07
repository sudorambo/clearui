# Data Model: Review Fixes

**Branch**: `012-review-fixes` | **Date**: 2026-03-07

## Modified Entities

### cui_a11y_entry (modified)

**Current**:
```c
typedef struct cui_a11y_entry {
    cui_a11y_role role;
    const char   *label;
    const char   *state;   // points to string literal — mutually exclusive
    float        x, y, w, h;
} cui_a11y_entry;
```

**Proposed**:
```c
typedef struct cui_a11y_entry {
    cui_a11y_role role;
    const char   *label;
    char          state[32]; // composable: "checked focused", "checked", "focused", ""
    float         x, y, w, h;
} cui_a11y_entry;
```

**Changes**: `const char *state` → `char state[32]`. Allows composition of multiple state tokens. 32 bytes supports all foreseeable combinations.

**Validation**: `state` is built by appending space-separated tokens. Buffer is zero-initialized; overflow is silently truncated (last token that fits wins).

---

### cui_text_input_opts (modified)

**Current**:
```c
typedef struct cui_text_input_opts { int placeholder; } cui_text_input_opts;
```

**Proposed**:
```c
typedef struct cui_text_input_opts { const char *placeholder; } cui_text_input_opts;
```

**Changes**: `int placeholder` → `const char *placeholder`. Semantically correct type.

**Validation**: NULL is valid (means no placeholder). The widget stores the pointer; lifetime must outlive the frame (typically a string literal).

---

### cui_cmd_type enum (modified style)

**Current**: `enum cui_cmd_type { ... };` (no typedef)

**Proposed**: `typedef enum cui_cmd_type { ... } cui_cmd_type;`

**Changes**: Add typedef for consistency with `cui_a11y_role`.

---

### cui_node_type enum (modified style)

**Current**: `enum cui_node_type { ... };` (no typedef)

**Proposed**: `typedef enum cui_node_type { ... } cui_node_type;`

**Changes**: Add typedef for consistency.

---

## New Functions

### cui_draw_buf_push_rounded_rect

```c
int cui_draw_buf_push_rounded_rect(cui_draw_command_buffer *buf,
    float x, float y, float w, float h, float r, unsigned int color);
```

Returns 0 on success, -1 if buffer is full. Emits `CUI_CMD_ROUNDED_RECT`.

---

## Modified Function Signatures

### cui_set_platform (clearui.h)

**Current**: `void cui_set_platform(cui_ctx *ctx, const void *platform, void *platform_ctx);`

**Proposed**: `void cui_set_platform(cui_ctx *ctx, const cui_platform *platform, cui_platform_ctx *platform_ctx);`

### cui_set_rdi (clearui.h)

**Current**: `void cui_set_rdi(cui_ctx *ctx, const void *rdi, void *rdi_ctx);`

**Proposed**: `void cui_set_rdi(cui_ctx *ctx, const cui_rdi *rdi, cui_rdi_context *rdi_ctx);`

Forward declarations of `cui_platform`, `cui_rdi`, `cui_platform_ctx`, and `cui_rdi_context` added to `clearui.h`.
