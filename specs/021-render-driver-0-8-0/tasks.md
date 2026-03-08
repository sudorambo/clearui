# Tasks: Render Driver (0.8.0)

**Input**: Design documents from `specs/021-render-driver-0-8-0/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1–US5)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Confirm build and RDI contract; no new repo structure.

- [x] T001 Verify draw command types (CUI_CMD_RECT, ROUNDED_RECT, LINE, TEXT, SCISSOR) and buffer format in `src/core/draw_cmd.h` and that `cui_render_submit` calls RDI submit/present in `src/core/render.c`
- [x] T002 [P] Confirm vendored stb_truetype and font atlas TTF path in `src/font/atlas.c` and `deps/` for reuse by soft RDI text rasterization

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Software RDI context, framebuffer allocation, and minimal submit path so later stories can add command handlers.

**Checkpoint**: submit() clears framebuffer and draws RECT; scissor state exists; present() is no-op until US3.

- [x] T003 Extend soft_ctx with framebuffer (unsigned char*), width, height, scissor rect (x,y,w,h); init allocates context in `src/rdi/clearui_rdi_soft.c`; shutdown frees framebuffer and context
- [x] T004 Add way to set framebuffer dimensions (e.g. extend init with optional width/height, or add set_viewport(rdi_ctx, w, h) and document in `specs/021-render-driver-0-8-0/contracts/rdi-software-0.8.md`); allocate or reallocate framebuffer when dimensions are set or change
- [x] T005 In submit: clear framebuffer at start; on CUI_CMD_SCISSOR set current scissor in soft_ctx; on CUI_CMD_RECT fill axis-aligned rectangle with color clipped to current scissor in `src/rdi/clearui_rdi_soft.c`
- [x] T006 Add RDI scissor helper: clip draw rect to current scissor (intersect); ensure all future draw commands use it in `src/rdi/clearui_rdi_soft.c`

---

## Phase 3: User Story 1 – Draw rects, rounded rects, lines, text (P1) – MVP

**Goal**: Software RDI rasterizes RECT, ROUNDED_RECT, LINE, and TEXT so the UI is visible without a GPU backend.

**Independent Test**: Build with soft RDI, run one frame with buttons/labels; read back framebuffer and assert non-zero pixels where widgets are drawn.

- [x] T007 [P] [US1] Implement CUI_CMD_ROUNDED_RECT in submit (fill rounded rect, clip to scissor) in `src/rdi/clearui_rdi_soft.c`
- [x] T008 [P] [US1] Implement CUI_CMD_LINE in submit (line segment with thickness, clip to scissor) in `src/rdi/clearui_rdi_soft.c`
- [x] T009 [US1] Implement CUI_CMD_TEXT in submit: rasterize glyphs with stb_truetype (e.g. stbtt_GetCodepointBitmap) or reuse atlas font data; blend at (x,y) with color; clip to scissor in `src/rdi/clearui_rdi_soft.c`
- [x] T010 [US1] Wire font for TEXT: ensure soft RDI can load same TTF as atlas (e.g. deps path) or share font from atlas; handle missing font (skip text or fallback) in `src/rdi/clearui_rdi_soft.c`

---

## Phase 4: User Story 2 – Scissor clipping (P2)

**Goal**: Scroll regions and overflow are correctly clipped.

**Independent Test**: Submit buffer with scissor then rect; assert pixels outside scissor are not written.

- [x] T011 [US2] Ensure all draw commands (RECT, ROUNDED_RECT, LINE, TEXT) are clipped to current scissor; scissor rect clipped to framebuffer bounds in `src/rdi/clearui_rdi_soft.c`
- [x] T012 [US2] Honor RDI scissor() callback if called (set same scissor state as CUI_CMD_SCISSOR) in `src/rdi/clearui_rdi_soft.c`

---

## Phase 5: User Story 3 – Present framebuffer to window (P3)

**Goal**: Rendered frame is visible on screen when platform supports it.

**Independent Test**: Run app with soft RDI + SDL3 adapter with present_software; see window with UI.

- [x] T013 Add optional present_software(platform_ctx, rgba, width, height, pitch_bytes) to `cui_platform` in `include/clearui_platform.h`; document in `specs/021-render-driver-0-8-0/contracts/platform-present-software-0.8.md`
- [x] T014 Extend soft RDI to accept platform reference: init optional params or setter (e.g. set_platform_target) so present() can call present_software; implement present() to call platform->present_software when set in `src/rdi/clearui_rdi_soft.c`
- [x] T015 [P] [US3] Implement present_software in `src/platform/cui_platform_sdl3.c` (blit RGBA to window; e.g. SDL_UpdateTexture + render or lock surface + memcpy); leave NULL in `src/platform/cui_platform_stub.c`
- [x] T016 [US3] Wire SDL3 adapter and soft RDI in demo or example so present_software is used when both are set; document in README or `specs/021-render-driver-0-8-0/quickstart.md`

---

## Phase 6: User Story 4 – Visual regression tests (P4)

**Goal**: Rendering changes are detected via baseline comparison.

**Independent Test**: Run test; it creates context with soft RDI, builds fixed UI, submit, compares framebuffer to baseline (checksum or sampled pixels).

- [x] T017 [US4] Add test that creates context with soft RDI, sets dimensions, builds fixed UI (known widgets), runs layout and draw, calls submit, reads framebuffer and compares to baseline (checksum or pixel sample) in `tests/unit/test_rdi_soft.c` or `tests/integration/test_visual_regression.c`
- [x] T018 [US4] Add baseline (expected checksum or reference path) and document how to update when rendering intentionally changes in `tests/baselines/` or test file comments; add test to Makefile and CI in `Makefile`

---

## Phase 7: User Story 5 – Default TTF font or docs (P5)

**Goal**: Text renders out of the box or is easy to configure.

**Independent Test**: New clone can build and run demo with visible text, or README clearly states font path and fallback.

- [x] T019 [US5] Document default TTF path (e.g. deps/default_font.ttf), fallback when missing, and how to provide own font in `README.md` and/or `deps/README.md`
- [x] T020 [P] [US5] Optionally bundle a minimal OFL-licensed TTF in `deps/` so clone-build-run shows text without user action; or confirm doc-only satisfies spec

---

## Phase 8: Polish & Cross-Cutting

**Purpose**: Docs, quickstart validation, and CI.

- [ ] T021 [P] Update README with software RDI usage, viewport/dimensions wiring, and present_software per `specs/021-render-driver-0-8-0/quickstart.md`
- [x] T022 Ensure Makefile and CI run new RDI/visual regression test; run quickstart steps to validate

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies.
- **Phase 2 (Foundational)**: Depends on Phase 1. Blocks all user stories.
- **Phase 3 (US1)**: Depends on Phase 2. MVP.
- **Phase 4 (US2)**: Depends on Phase 2 (scissor state); can overlap with US1 (T011–T012 build on T005–T006).
- **Phase 5 (US3)**: Depends on Phase 2 (present path); independent of US1/US2 after T014.
- **Phase 6 (US4)**: Depends on Phase 2 and at least US1 (submit draws); baseline from known UI.
- **Phase 7 (US5)**: Independent; can run in parallel with US3/US4.
- **Phase 8 (Polish)**: After all stories.

### User Story Dependencies

- **US1 (P1)**: After Foundational. No dependency on other stories.
- **US2 (P2)**: After Foundational; uses same submit loop and scissor from Phase 2.
- **US3 (P3)**: After Foundational; platform extension and soft RDI present wiring.
- **US4 (P4)**: After Foundational and US1 (so framebuffer has content to compare).
- **US5 (P5)**: Independent (docs or deps asset).

### Parallel Opportunities

- T002, T007, T008 can run in parallel with other tasks in their phases.
- T015, T019, T020, T021 are [P] where no same-file work is in progress.
- US3 (T015) and US5 (T019/T020) can proceed in parallel after foundational.

---

## Implementation Strategy

### MVP First (User Story 1)

1. Phase 1 → Phase 2 → Phase 3.
2. Validate: run app with soft RDI, set dimensions, one frame with buttons/labels; inspect or checksum framebuffer.
3. Then add US2 (scissor), US3 (present), US4 (regression test), US5 (font docs).

### Incremental Delivery

1. Setup + Foundational → submit draws RECT, scissor state ready.
2. US1 → rects, rounded rects, lines, text visible.
3. US2 → clipping correct.
4. US3 → window shows frame with SDL3 + present_software.
5. US4 → CI regression test.
6. US5 → font doc or bundled TTF.
7. Polish → README and quickstart.

---

## Task Summary

| Phase   | Story | Task count |
|---------|-------|------------|
| 1 Setup | –     | 2          |
| 2 Foundational | – | 4 |
| 3 US1  | US1   | 4          |
| 4 US2  | US2   | 2          |
| 5 US3  | US3   | 4          |
| 6 US4  | US4   | 2          |
| 7 US5  | US5   | 2          |
| 8 Polish | –   | 2          |
| **Total** |     | **22**     |

**Format validation**: All tasks use `- [ ] [Txxx] [P?] [USx?] Description with file path(s)`.
