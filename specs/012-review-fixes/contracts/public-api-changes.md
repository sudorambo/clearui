# Public API Changes: Review Fixes

**Branch**: `012-review-fixes` | **Date**: 2026-03-07

## Breaking Changes

### 1. `cui_set_platform` signature change

```c
// Before (accepts any pointer)
void cui_set_platform(cui_ctx *ctx, const void *platform, void *platform_ctx);

// After (type-safe)
void cui_set_platform(cui_ctx *ctx, const cui_platform *platform, cui_platform_ctx *platform_ctx);
```

**Migration**: No code change required if callers were already passing correct types. If callers were casting from unrelated types, they will get a compiler warning (desired behavior).

### 2. `cui_set_rdi` signature change

```c
// Before
void cui_set_rdi(cui_ctx *ctx, const void *rdi, void *rdi_ctx);

// After
void cui_set_rdi(cui_ctx *ctx, const cui_rdi *rdi, cui_rdi_context *rdi_ctx);
```

**Migration**: Same as above.

### 3. `cui_text_input_opts.placeholder` type change

```c
// Before
typedef struct cui_text_input_opts { int placeholder; } cui_text_input_opts;

// After
typedef struct cui_text_input_opts { const char *placeholder; } cui_text_input_opts;
```

**Migration**: Any code using `.placeholder = 0` (zero-init) continues to work since `0` for a pointer is NULL. Code using `.placeholder = 1` or similar integer values must be updated to a string.

## Additive Changes (Non-Breaking)

### 4. New function: `cui_draw_buf_push_rounded_rect`

```c
int cui_draw_buf_push_rounded_rect(cui_draw_command_buffer *buf,
    float x, float y, float w, float h, float r, unsigned int color);
```

### 5. New forward declarations in `clearui.h`

```c
typedef struct cui_platform cui_platform;
typedef struct cui_platform_ctx cui_platform_ctx;
typedef struct cui_rdi cui_rdi;
typedef struct cui_rdi_context cui_rdi_context;
```

## Behavioral Changes (Same API, Different Behavior)

### 6. `cui_draw_circle` emits rounded rect instead of rect

Before: Emitted `CUI_CMD_RECT` (a square).
After: Emits `CUI_CMD_ROUNDED_RECT` with `r = radius` (visually a circle).

### 7. `cui_a11y_entry.state` is now a char array

Before: `const char *state` pointing to a string literal.
After: `char state[32]` containing a composed state string.

Callers reading `entry->state` as a `const char *` will still work (array decays to pointer). Callers copying the struct will now copy the buffer (32 bytes instead of 8 bytes for a pointer).

### 8. `cui_arena_alloc_aligned` validates alignment

Before: Invalid alignment (0, non-power-of-2) caused undefined behavior.
After: Returns NULL for invalid alignment.
