# Tasks: Platform Backend (0.7.0)

**Input**: Design documents from `specs/020-platform-backend-0-7-0/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: FR-003 requires an integration test that opens a real window, renders one frame, and closes; test MAY skip when no display. FR-004 requires documentation for custom adapters.

**Organization**: Tasks grouped by phase: interface extension, SDL3 adapter implementation, build, integration test, documentation. US1 = real window + one frame; US2 = full capabilities (clipboard, cursor, scale); US3 = bring your own platform docs.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `include/`, `src/`, `tests/` at repository root

---

## Phase 1: Setup (Verification)

**Purpose**: Confirm design is loadable.

- [x] T001 Verify `specs/020-platform-backend-0-7-0/spec.md` and `plan.md` are present and align with ROADMAP Milestone 6 (Platform Backend 0.7.0).

---

## Phase 2: Platform interface (Blocking for adapter)

**Purpose**: Optional scale factor and poll_events with ctx so adapters can inject input. Stub and ClearUI call site must be updated.

**Checkpoint**: `clearui_platform.h` has optional `scale_factor_get` and `poll_events` takes `cui_ctx *`; stub and `context.c` updated.

- [x] T002 Add optional `float (*scale_factor_get)(cui_platform_ctx *ctx)` to `struct cui_platform` in `include/clearui_platform.h` (NULL = not supported). Document that return value must be >= 1.0. Do not add to stub (stub keeps NULL for optional slots).
- [x] T003 Change `poll_events` signature in `include/clearui_platform.h` from `bool (*poll_events)(cui_platform_ctx *ctx)` to `bool (*poll_events)(cui_platform_ctx *ctx, void *cui_ctx)` so adapters can call ClearUI inject APIs. Document that second argument is `cui_ctx *` (void* to avoid circular include); NULL when not set. Update `src/platform/cui_platform_stub.c`: poll_events(platform_ctx, cui_ctx_ignored) — ignore second arg, return true. Update `src/core/context.c`: call `ctx->platform->poll_events(ctx->platform_ctx, ctx)` instead of `poll_events(ctx->platform_ctx)`.

---

## Phase 3: SDL3 adapter — core (US1)

**Purpose**: Implement required platform callbacks so a window can be created, one frame run, and window destroyed.

**Checkpoint**: `cui_platform_sdl3.c` compiles (when SDL3 available), implements window_create, window_destroy, window_get_size, poll_events; exports `const cui_platform *cui_platform_sdl3_get(void)`.

- [x] T004 [US1] Create `src/platform/cui_platform_sdl3.c`: Define private struct (e.g. SDL_Window*, window size cache) cast as `cui_platform_ctx`. Implement `window_create`: init SDL3 if needed, create window, allocate context struct, set *out_ctx, return 0 on success and non-zero on failure (do not set *out_ctx on failure). Implement `window_destroy`: free context and destroy window; no-op if ctx NULL. Implement `window_get_size`: write current window size (e.g. from SDL_GetWindowSize) into *width, *height. Implement `poll_events`: loop SDL_PollEvent; on quit event return false; on mouse move call `cui_inject_mouse_move((cui_ctx*)cui_ctx, x, y)`; on mouse button call `cui_inject_click`; on scroll call `cui_inject_scroll`; on key/text call `cui_inject_key`/`cui_inject_char` as appropriate; return true. Include `clearui.h` or minimal decls for inject functions to avoid circular include. Declare `const cui_platform *cui_platform_sdl3_get(void)` and fill static `cui_platform` with all required pointers and NULL for optional ones initially.
- [x] T005 [US1] Add `include/clearui_platform_sdl3.h` (or declare in existing header) with `const cui_platform *cui_platform_sdl3_get(void)` so apps and the integration test can link the SDL3 adapter.

---

## Phase 4: SDL3 adapter — optional capabilities (US2)

**Purpose**: Clipboard, cursor, scale factor so the adapter is fully capable where SDL3 supports it.

- [x] T006 [US2] In `src/platform/cui_platform_sdl3.c`: Implement `clipboard_get` and `clipboard_set` using SDL3 clipboard API; assign to the static platform struct. Implement `cursor_set(ctx, shape)`: map shape (e.g. 0 = default arrow, 1 = text caret) to SDL cursor and set. Implement `scale_factor_get(ctx)`: return SDL display/window scale factor (>= 1.0). If any capability is not available on the platform, leave pointer NULL or return a safe default (e.g. scale 1.0). Define cursor shape constants in `clearui_platform.h` if not already (e.g. CUI_CURSOR_DEFAULT, CUI_CURSOR_TEXT).

---

## Phase 5: Build system

**Purpose**: Optional SDL3 build; default remains stub-only.

- [x] T007 Update `Makefile`: Add optional object `src/platform/cui_platform_sdl3.o` built only when WITH_SDL3=1 or when `sdl3-config --cflags` (or equivalent) succeeds. Add SDL3 CFLAGS and LDFLAGS via sdl3-config or pkg-config. Default `all` and `$(OBJS)` must not require SDL3 (stub-only). Document in README or Makefile comment: set WITH_SDL3=1 to build the SDL3 adapter; link apps that use it with the SDL3 adapter object and SDL3.
- [ ] T008 Ensure existing targets (unit-tests, integration-tests that do not use a real window) still build and run without SDL3. New integration test that uses the SDL3 adapter is built only when WITH_SDL3=1 (or similar).

---

## Phase 6: Integration test (US1)

**Purpose**: FR-003 — open real window, one frame, close; skip when no display.

- [x] T009 [US1] Create `tests/integration/test_platform_window.c`: If WITH_SDL3 (or runtime check for display) is not available, print "test_platform_window: SKIP (no display or SDL3)" and exit 0. Otherwise: get platform via `cui_platform_sdl3_get()`, call `window_create(&platform_ctx, "test", 400, 300)`. If creation fails (e.g. no display), print SKIP and exit 0. Create `cui_ctx` with cui_create, call cui_set_platform(ctx, platform, platform_ctx), cui_begin_frame, build minimal UI (e.g. cui_center, cui_label("OK"), cui_end, cui_end), cui_end_frame, then window_destroy(platform_ctx), cui_destroy(ctx). Assert no crash. Print "test_platform_window: PASS" and exit 0.
- [x] T010 Add `test_platform_window` target to Makefile when WITH_SDL3=1; add to integration-tests phony only when WITH_SDL3=1, or run it as optional (e.g. `make integration-tests` runs it only if built; if not built, skip). Add binary to `clean` target.

---

## Phase 7: Documentation (US3)

**Purpose**: FR-004 — how to implement and register a custom platform adapter.

- [x] T011 [P] [US3] Add a "Bring your own platform" (or "Platform adapter") section to `README.md` (or create `docs/platform-adapter.md`): List required `cui_platform` callbacks (window_create, window_destroy, window_get_size, poll_events); list optional (clipboard_get/set, cursor_set, surface_get/surface_destroy, scale_factor_get). Explain that poll_events receives (platform_ctx, cui_ctx) and the adapter should call cui_inject_mouse_move, cui_inject_click, cui_inject_scroll, cui_inject_key, cui_inject_char as events occur; return false to quit. Document cui_set_platform(ctx, &my_platform, my_platform_ctx) before first frame. Reference `specs/020-platform-backend-0-7-0/contracts/platform-adapter-0.7.md` and quickstart.

---

## Phase 8: Polish & validation

**Purpose**: Full build and test pass; CI note.

- [x] T012 Run `make clean && make all && make unit-tests` (and existing integration-tests that do not require a display) and confirm all pass. If WITH_SDL3=1, run `test_platform_window` and confirm it passes or skips. Document in README or CI that the "real window" integration test may be skipped in headless environments.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1**: No dependencies.
- **Phase 2**: Depends on Phase 1. T002 (scale_factor_get) and T003 (poll_events signature) can be done in order; both are needed before the adapter implements poll_events with injection.
- **Phase 3 (US1 adapter core)**: Depends on Phase 2. T004 implements the SDL3 file; T005 exposes the getter.
- **Phase 4 (US2 capabilities)**: Depends on Phase 3. T006 extends the same file.
- **Phase 5 (Build)**: Depends on Phase 3 (adapter file exists). T007–T008.
- **Phase 6 (Integration test)**: Depends on Phase 5 (SDL3 build) and Phase 3 (adapter). T009–T010.
- **Phase 7 (Docs)**: Can run in parallel with Phase 5–6 once interface is stable (after Phase 2). T011.
- **Phase 8**: Depends on Phases 5–7.

### Parallel Opportunities

- T002 and T003 both touch the platform interface; do T002 then T003 (or together in one change).
- T011 (documentation) can be done in parallel with T007–T010 once T002–T003 are done.

### Implementation Strategy

1. **Phase 1**: T001.
2. **Phase 2**: T002 → T003 (interface + stub + context.c).
3. **Phase 3**: T004 → T005 (SDL3 adapter core + header).
4. **Phase 4**: T006 (optional capabilities in same file).
5. **Phase 5**: T007 → T008 (Makefile).
6. **Phase 6**: T009 → T010 (integration test + Makefile).
7. **Phase 7**: T011 (docs).
8. **Phase 8**: T012 (full validation).

---

## Notes

- **poll_events and injection**: ClearUI calls `poll_events(platform_ctx, ctx)` from `cui_begin_frame`. The second parameter allows the adapter to call `cui_inject_*` so the app does not need to handle raw events. Using `void*` for the second argument avoids including `clearui.h` in `clearui_platform.h` (adapter can cast to `cui_ctx*` internally).
- **Cursor shapes**: If `clearui_platform.h` does not define cursor shape constants, add e.g. `CUI_CURSOR_DEFAULT`, `CUI_CURSOR_TEXT` so the adapter and ClearUI (e.g. when setting cursor over text input) agree.
- **Stub**: After T003, the stub’s poll_events signature takes two arguments; ignore the second and return true so all existing tests still pass.
