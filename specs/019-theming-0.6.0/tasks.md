# Tasks: Theming (0.6.0)

**Input**: Design documents from `specs/019-theming-0.6.0/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Spec FR-006 and User Story 1 require a unit test that applies a theme and asserts draw command colors change.

**Organization**: Tasks grouped by user story (US1 = set theme at runtime, US2 = dark preset, US3 = draw/layout use theme from context). Foundation provides struct, context storage, and accessor.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `include/`, `src/`, `tests/` at repository root

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Confirm design is loadable.

- [x] T001 Verify `specs/019-theming-0.6.0/spec.md` and `plan.md` are present and align with ROADMAP Milestone 5 (Theming 0.6.0).

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Public theme struct, context theme slot and default, internal accessor. Required before draw/layout can read theme.

**Checkpoint**: `cui_theme` declared, context has theme field initialized to default, `cui_ctx_theme(ctx)` returns current theme.

- [x] T002 Define `cui_theme` struct in `include/clearui.h` with fields per `specs/019-theming-0.6.0/contracts/theme-api.md`: `text_color`, `button_bg`, `checkbox_bg`, `input_bg`, `corner_radius`, `font_size`, `focus_ring_color`, `focus_ring_width` (all types as in contract). Declare `void cui_set_theme(cui_ctx *ctx, const cui_theme *theme)` and `void cui_theme_dark(cui_theme *out)` in `include/clearui.h`.
- [x] T003 Add `cui_theme theme` field to `struct cui_ctx` in `src/core/context.c`. Define the built-in default theme (same values as current `src/core/theme.h` #defines). In `cui_create`, initialize `ctx->theme` to that default. Implement `cui_set_theme(ctx, theme)`: if `theme` non-NULL copy `*theme` into `ctx->theme`, else copy built-in default into `ctx->theme`; if `ctx` is NULL do nothing.
- [x] T004 Add internal accessor `const cui_theme *cui_ctx_theme(cui_ctx *ctx)` in `src/core/context.h` and implement in `src/core/context.c` (return `ctx ? &ctx->theme : NULL` or a pointer to a static default when ctx is NULL so it is never NULL). Document that draw_cmd and layout use this to read current theme.

---

## Phase 3: User Story 3 — Draw and layout use theme from context (Priority: P1)

**Goal**: All runtime theme reads go through context; no CUI_THEME_* macros used at draw/layout time.

**Independent Test**: After T006–T007, run existing unit tests (default theme); then US1 test (custom theme) verifies draw colors.

### Implementation for User Story 3

- [x] T005 [US3] Update `src/core/draw_cmd.c`: add context parameter to `cui_build_draw_from_tree` if not already present (it already receives `ctx`). Replace every use of `CUI_THEME_DEFAULT_TEXT_COLOR`, `CUI_THEME_DEFAULT_BUTTON_BG`, `CUI_THEME_DEFAULT_CHECKBOX_BG`, `CUI_THEME_DEFAULT_INPUT_BG`, `CUI_THEME_FOCUS_RING_COLOR`, `CUI_THEME_FOCUS_RING_WIDTH` with reads from `cui_ctx_theme(ctx)` (e.g. `cui_ctx_theme(ctx)->button_bg`). Include `context.h` if needed. When `cui_ctx_theme(ctx)` is NULL use current `theme.h` values as fallback so standalone callers do not break.
- [x] T006 [US3] Update `src/layout/layout.c`: change signature to `void cui_layout_run(cui_ctx *ctx, cui_node *root, float viewport_w, float viewport_h)`. Replace uses of `CUI_THEME_DEFAULT_FONT_SIZE` with a font size from context: when `ctx` non-NULL use `cui_ctx_theme(ctx)->font_size`, when NULL use 16 (or include `theme.h` and use `CUI_THEME_DEFAULT_FONT_SIZE`). Update `src/layout/layout.h` and all call sites: `src/core/context.c` (pass `ctx`), `tests/unit/test_layout.c` (pass NULL so layout uses default font size).
- [x] T007 [US3] Update `src/core/draw_cmd.c` so that `node_text_color` (or equivalent) and any remaining theme references use `cui_ctx_theme(ctx)`; ensure no compile-time macro is used at runtime in the draw path. Verify `theme.h` is only used for default initializer or fallback when ctx is NULL.

---

## Phase 4: User Story 1 — Set theme at runtime (Priority: P1) — MVP

**Goal**: App can call `cui_set_theme(ctx, &theme)` and the next frame’s draw and layout use that theme.

**Independent Test**: Create ctx, call `cui_set_theme(ctx, &custom_theme)` with distinct `button_bg`, build frame with button, `cui_end_frame`, inspect draw buffer and assert a fill_rect command has the custom theme’s button_bg color.

### Implementation for User Story 1

- [x] T008 [US1] Ensure `cui_set_theme` is declared in `include/clearui.h` and implemented in `src/core/context.c` (done in T003; this task is a verification checkpoint). No additional code if T003 is complete; otherwise add declaration and implementation.
- [x] T009 [US1] Create `tests/unit/test_theme.c`: create context with stub platform/RDI, set a custom theme with a distinct `button_bg` (e.g. `0xff112233`), `cui_begin_frame`, build a simple UI with a button (e.g. `cui_center`, `cui_button`), `cui_end`, `cui_end_frame`; get draw buffer via `cui_ctx_draw_buf(ctx)`, find a `CUI_CMD_RECT`/fill_rect with color equal to the custom theme’s `button_bg`, assert found. Test `cui_set_theme(ctx, NULL)` resets to default (optional second assertion or separate run). Print "PASS" and exit 0.
- [x] T010 [US1] Add `test_theme` target to `Makefile` (pattern like existing unit tests), add it to the `unit-tests` phony dependency and recipe, and add the binary to the `clean` target’s `rm -f` list.

---

## Phase 5: User Story 2 — Dark theme preset (Priority: P2)

**Goal**: Library provides a one-call dark theme; app can apply it and see dark UI.

**Independent Test**: Call `cui_theme_dark(&t)`, `cui_set_theme(ctx, &t)`, build frame, assert draw buffer uses dark background color (e.g. button_bg dark).

### Implementation for User Story 2

- [x] T011 [P] [US2] Implement `void cui_theme_dark(cui_theme *out)` in `src/core/context.c` (or a new `src/core/theme.c` if preferred): fill `*out` with dark theme (e.g. dark gray/black backgrounds, light text, same focus_ring_color/width for WCAG). Declare in `include/clearui.h`. Do not allocate; only assign struct fields.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: theme.h role, docs, full validation.

- [x] T012 Update `src/core/theme.h`: retain only the default theme initializer or #defines used to initialize the built-in default (e.g. for `context.c` or a single default struct). Ensure no consumer uses these macros at runtime in the draw/layout path; all runtime reads go through `cui_ctx_theme(ctx)`. Add brief comment that runtime theme is set via `cui_set_theme` and default is built-in.
- [x] T013 Update `README.md` (or public docs): add a short subsection on theming — `cui_theme`, `cui_set_theme(ctx, &theme)`, `cui_set_theme(ctx, NULL)` to reset, and `cui_theme_dark(&t)` for dark preset, per `specs/019-theming-0.6.0/quickstart.md`.
- [x] T014 Run `make clean && make all && make unit-tests` and confirm all tests pass, including the new `test_theme` and existing tests (default theme behavior unchanged).

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1**: No dependencies.
- **Phase 2**: Depends on Phase 1. T002 (struct + decls) first; T003 (context storage + set_theme impl) and T004 (accessor) can follow; T004 is needed by T005–T007.
- **Phase 3 (US3)**: Depends on Phase 2. T005 and T006–T007 update draw_cmd and layout to use `cui_ctx_theme(ctx)`. T006 changes layout_run signature — do before or with T007; context.c and test_layout.c must be updated to pass ctx (or NULL).
- **Phase 4 (US1)**: Depends on Phase 3 (draw must read theme for US1 test to pass). T008 is verification; T009–T010 add test.
- **Phase 5 (US2)**: Depends on Phase 2 (cui_theme_dark declared in T002). T011 can be done in parallel with Phase 3/4 once T002 is done.
- **Phase 6**: Depends on Phases 3–5.

### User Story Dependencies

- **US3 (P1)**: After Phase 2. Enables US1 (draw/layout use theme so set_theme has effect).
- **US1 (P1)**: After US3. Delivers set_theme + test.
- **US2 (P2)**: After Phase 2 (decl in clearui.h). Independent of US3/US1.

### Parallel Opportunities

- **Phase 2**: T003 and T004 are sequential (both touch context.c). T002 is first (header).
- **Phase 5**: T011 (cui_theme_dark) can be implemented in parallel with test work (T009–T010) once T002–T004 are done.

### Implementation Strategy

1. **Phase 1**: T001.
2. **Phase 2**: T002 → T003 → T004 (struct, storage, setter, accessor).
3. **Phase 3**: T005 (draw_cmd use theme), T006 (layout_run take ctx, use theme font_size), T007 (verify no macro at runtime in draw).
4. **Phase 4**: T008 (verify set_theme), T009 (test_theme.c), T010 (Makefile).
5. **Phase 5**: T011 (cui_theme_dark).
6. **Phase 6**: T012 (theme.h), T013 (README), T014 (full make + tests).

---

## Notes

- Layout currently has no `ctx`; adding `ctx` to `cui_layout_run` is required so layout can read `cui_ctx_theme(ctx)->font_size`. Unit test `test_layout.c` builds its own tree without a full context — pass `NULL` for `ctx` and use default font size (16 or theme.h #define) when `ctx` is NULL.
- draw_cmd.c already receives `ctx` in `cui_build_draw_from_tree(ctx, root, buf, ox, oy)`; no signature change needed there.
- Default theme numeric values must match current `theme.h` so that when no theme is set, behavior and appearance are unchanged.
