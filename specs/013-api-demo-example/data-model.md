# Data Model: Full API Demo Example

**Branch**: `013-api-demo-example` | **Date**: 2026-03-07

## Overview

This feature introduces no new data types or entities. It consumes the existing ClearUI public API. The "data model" here is the set of demo-local state used during the example's 3-frame run.

## Demo State

| Variable | Type | Lifetime | Purpose |
|----------|------|----------|---------|
| `ctx` | `cui_ctx *` | program | ClearUI context |
| `plat_ctx` | `cui_platform_ctx *` | program | Stub platform context |
| `rdi_ctx` | `cui_rdi_context *` | program | Software RDI context |
| `counter` | `int *` (via `cui_state`) | persistent (Vault) | Demonstrates cross-frame state |
| `checked` | `int *` (via `cui_state`) | persistent (Vault) | Checkbox state |
| `text_buf` | `char[64]` | stack | Text input buffer |
| `formatted` | `const char *` (via `cui_frame_printf`) | frame | Demonstrates frame allocator |
| `frame_mem` | `void *` (via `cui_frame_alloc`) | frame | Demonstrates raw frame allocation |

## Relationships

```
cui_ctx
├── cui_platform (stub) + cui_platform_ctx
├── cui_rdi (soft) + cui_rdi_context
├── Vault
│   ├── "counter" → int
│   └── "checked" → int
├── Arena (UI tree, reset each frame)
└── Frame allocator (transient strings/memory, reset each frame)
```

## New Files

| Path | Purpose |
|------|---------|
| `examples/demo.c` | Full API demo (~150–200 lines) |

## Modified Files

| Path | Change |
|------|--------|
| `Makefile` | Add `demo` target |
| `README.md` | Add "Examples" section with link to `examples/demo.c` |
