# Tasks: Refactor `cui_center` to use `push_container`

**Input**: Design documents from `/specs/006-refactor-cui-center/`
**Prerequisites**: plan.md, spec.md, research.md, quickstart.md

## Format: `[ID] [P?] Description`

- **[Rn]**: Which requirement this task satisfies (R1–R4 from spec.md)
- Include exact file paths in descriptions

---

## Phase 1: Implementation (R1, R2)

**Purpose**: Replace duplicated container-setup code in `cui_center` with a `push_container` delegation

- [x] T001 [R1] Replace the body of `cui_center` in `src/widget/layout.c` with `push_container(ctx, CUI_NODE_CENTER, NULL, 400, 300)` — remove the manual `cui_ctx_root_ptr`, `cui_node_alloc`, tree-attachment, and `cui_ctx_push_parent` calls; keep the `if (!ctx) return;` guard

---

## Phase 2: Verification (R2, R3, R4)

**Purpose**: Confirm API unchanged, no regressions, and duplication eliminated

- [x] T002 [R2] Verify `void cui_center(cui_ctx *ctx)` signature in `include/clearui.h` is unchanged — no code change, just confirm
- [x] T003 [R3] Run `make clean && make all && make unit-tests && make integration-tests` and confirm zero warnings and all tests PASS
- [x] T004 [R4] Run `grep -c 'cui_ctx_root_ptr\|cui_node_alloc\|cui_ctx_push_parent' src/widget/layout.c` and confirm each count is exactly 1 (only inside `push_container`, not duplicated in `cui_center`)

---

## Dependencies & Execution Order

- **Phase 1**: No prerequisites — start immediately
- **Phase 2**: Depends on Phase 1 completion
  - T002, T003, T004 can run in parallel (all are read-only verification)

---

## Implementation Strategy

### Execution Order

1. T001 (single code change)
2. T002 + T003 + T004 in parallel (verification)

### Acceptance Criteria (from spec.md)

- `cui_center` body is exactly: `if (!ctx) return; push_container(ctx, CUI_NODE_CENTER, NULL, 400, 300);`
- `make clean && make all && make unit-tests && make integration-tests` passes with zero warnings
- `cui_ctx_root_ptr`, `cui_node_alloc`, `cui_ctx_push_parent` each appear exactly once in `layout.c` (inside `push_container` only)
