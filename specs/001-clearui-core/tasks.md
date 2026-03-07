# Tasks: ClearUI Core

**Input**: Design documents from `specs/001-clearui-core/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Organization**: Tasks are grouped by implementation phase and user story (US1–US6) derived from the ClearUI RFC. Each phase is independently testable.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1–US6) for implementation phases
- Include exact file paths in descriptions

## Path Conventions

- **Repository root**: `include/`, `src/`, `tests/` (per plan.md)
- **Headers**: `include/clearui.h`, `include/clearui_rdi.h`, `include/clearui_platform.h`
- **Source**: `src/core/`, `src/layout/`, `src/widget/`, `src/rdi/`, `src/platform/`, `src/font/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and directory structure per plan.md

- [x] T001 Create directory structure: include/, src/core/, src/layout/, src/widget/, src/rdi/, src/platform/, src/font/, tests/unit/, tests/integration/
- [x] T002 Add minimal build configuration in repo root (Makefile or single-command cc) for C11 with -std=c11 -Wall -Wextra -Wpedantic
- [x] T003 [P] Add .gitignore for build artifacts and optional C/C++ editor config (e.g. clang-format) in repo root

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before any user story. RDI and platform interfaces, context, arena, frame allocator, vault.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete.

- [x] T004 Define RDI interface: include/clearui_rdi.h with struct of function pointers (init, shutdown, texture create/upload/destroy, submit command buffer, scissor, present) per contracts/rdi-interface.md
- [x] T005 Define platform interface: include/clearui_platform.h with cui_platform struct (window create/destroy, event poll, clipboard, cursor, GPU surface) per contracts/platform-interface.md
- [x] T006 Implement arena allocator in src/core/arena.c (grow-by-double, reset by rewind; default 4 MB) with declaration in src/core/arena.h
- [x] T007 Implement frame bump allocator in src/core/frame_alloc.c (reset each frame) with declaration in src/core/frame_alloc.h
- [x] T008 Implement Vault in src/core/vault.c (FNV-1a hash, key→blob, zero-init on first access, optional stale pruning) with declaration in src/core/vault.h
- [x] T009 Implement context skeleton in src/core/context.c: cui_ctx struct (opaque), cui_create/cui_destroy; wire in arena, frame alloc, vault; accept optional cui_config and platform/RDI pointers per data-model.md
- [x] T010 [P] Unit test for arena: alloc, reset, grow in tests/unit/test_arena.c
- [x] T011 [P] Unit test for vault: get/set by key, zero-init, same key same pointer in tests/unit/test_vault.c

**Checkpoint**: Foundation ready — context, memory model, and interfaces defined; unit tests for arena and vault pass.

---

## Phase 3: User Story 1 — Zero-to-Window (Priority: P1) 🎯 MVP

**Goal**: Runnable Hello World: create context, open window, one frame with one label, single-command build and run. (Constitution III “window with a button” is satisfied by end of US2 when button is implemented.)

**Independent Test**: Build with `cc main.c clearui.c ... -o hello`; run `./hello`; window appears with "Hello, World!" (or placeholder); close window exits cleanly.

- [x] T012 [US1] Define draw command buffer types (tagged union: rect, rounded_rect, line, text, scissor) in src/core/draw_cmd.h; no dynamic alloc per command
- [x] T013 [US1] Implement node descriptor and declared-tree build in src/core/node.c: minimal node type (e.g. label), append to current frame tree from arena; declare in src/core/node.h
- [x] T013b [US1] Implement tree diff in src/core/diff.c: compare retained tree with declared tree each frame; output minimal updates for layout/a11y/focus; retain tree for next frame; declaration in src/core/diff.h
- [x] T014 [US1] Implement begin_frame/end_frame in src/core/context.c: reset arena and frame alloc in begin_frame; in end_frame run diff (if implemented), then build draw list from current tree (single label → one draw command)
- [x] T015 [US1] Implement software RDI in src/rdi/clearui_rdi_soft.c: implement RDI function pointers; submit command buffer to in-memory or headless target for CI
- [x] T016 [US1] Implement SDL3 platform adapter in src/platform/cui_platform_sdl3.c: window create, event poll, running flag, surface for RDI; declaration in include/clearui_platform.h or src/platform/platform_sdl3.h
- [x] T017 [US1] Add public API in include/clearui.h: cui_create, cui_destroy, cui_running, cui_begin_frame, cui_end_frame; cui_center, cui_label, cui_end; cui_config; link to context and platform
- [x] T018 [US1] Implement cui_center and cui_label in src/widget/layout.c and src/widget/label.c (or unified widget layer) so one label produces one node and one draw command
- [x] T019 [US1] Integration test: run Hello World pipeline (context + SDL3 adapter + soft RDI) in tests/integration/test_hello.c; assert window opens and one frame completes

**Checkpoint**: Hello World runs; zero-to-window in under five minutes with single-command build.

---

## Phase 4: User Story 2 — Memory Model (Priority: P2)

**Goal**: cui_state and cui_frame_printf; Counter example works (persistent count, button click increments).

**Independent Test**: Run counter example from quickstart.md; count persists across frames; button increments value.

- [x] T020 [US2] Expose cui_state(ctx, key, size) in include/clearui.h and implement in src/core/context.c using vault; return pointer to persistent blob, zero-initialized on first use
- [x] T021 [US2] Expose cui_frame_alloc and cui_frame_printf in include/clearui.h; implement in src/core/context.c using frame allocator; document valid-until-next-begin_frame
- [x] T022 [US2] Add cui_button to include/clearui.h and implement in src/widget/button.c: return true when clicked; hit-test and event dispatch from end_frame
- [x] T023 [US2] Wire button and label in layout so counter example (cui_state, cui_button, cui_frame_printf, cui_label) runs per spec §5.3
- [x] T023b [US2] Add cui_run(ctx, ui_func) in include/clearui.h and implement in src/core/context.c: one-liner that runs event loop, sleeping, and event polling; calls ui_func each frame per spec §7.2

**Checkpoint**: Counter example runs; memory model (arena + frame + vault) fully usable from API; zero-to-window “with a button” satisfied.

---

## Phase 5: User Story 3 — Layout (Priority: P3)

**Goal**: Flexbox-style layout: cui_row, cui_column, cui_center, cui_layout (gap, padding, align); layout pass computes positions.

**Independent Test**: UI with row of buttons and column of labels lays out correctly; gap and alignment match cui_layout.

- [x] T024 [US3] Define cui_layout struct in include/clearui.h (gap, padding, padding_x, padding_y, max_width, min_width, max_height, flex, align, align_y) and CUI_ALIGN_* constants
- [x] T025 [US3] Implement layout engine in src/layout/layout.c: single pass over tree plus flex-grow pass; output per-node x, y, width, height (logical pixels); declaration in src/layout/layout.h
- [x] T026 [US3] Add cui_row, cui_column, cui_end to include/clearui.h; implement in src/widget/layout.c (or src/layout/container.c) building container nodes with cui_layout; wire to layout engine in end_frame
- [x] T027 [US3] Add cui_center to use layout engine to center single child; ensure layout runs in end_frame before draw command generation
- [x] T027b [US3] Add cui_stack to include/clearui.h and implement in src/widget/layout.c (or src/layout/container.c): overlay children at same position per spec §6.1
- [x] T027c [US3] Add cui_wrap to include/clearui.h and implement in src/layout/layout.c: row/column with wrapping to next line per spec §6.1
- [x] T028 [P] [US3] Unit test for layout math: row/column with gap and align in tests/unit/test_layout.c

**Checkpoint**: Layout layer complete; row/column/center/stack/wrap and cui_layout drive positioning.

---

## Phase 6: User Story 4 — Widgets & Style (Priority: P4)

**Goal**: checkbox, text_input, scroll, push_style/pop_style; todo-like example from spec §5.4 runs.

**Independent Test**: Todo list with text input, add button, scrollable list, checkboxes, delete buttons; styling (e.g. strikethrough when done) applies.

- [x] T029 [US4] Add cui_push_style and cui_pop_style in include/clearui.h; implement style stack in src/core/context.c (or src/widget/style.c); apply current style to newly declared nodes
- [x] T030 [US4] Add cui_checkbox(ctx, id, bool*) in include/clearui.h; implement in src/widget/checkbox.c; hit-test and toggle state
- [x] T031 [US4] Add cui_text_input(ctx, id, buffer, capacity, opts) in include/clearui.h; implement in src/widget/text_input.c; focus and keyboard input path
- [x] T032 [US4] Add cui_scroll with cui_scroll_opts (e.g. max_height) in include/clearui.h; implement in src/widget/scroll.c; scroll offset in layout and clip in draw
- [x] T033 [US4] Add cui_spacer and cui_icon_button (e.g. CUI_ICON_TRASH) to include/clearui.h; implement in src/widget/spacer.c and src/widget/icon_button.c
- [x] T034 [US4] Ensure label_styled and style (text_color, text_decoration) support todo example; wire draw commands for text styling
- [x] T034b [US4] Add canvas API per spec §5.5: cui_canvas(ctx, opts), cui_canvas_draw_list(ctx), cui_draw_rect, cui_draw_circle, cui_draw_text in include/clearui.h; implement in src/widget/canvas.c; draw list appends to frame command buffer in src/core/draw_cmd.h

**Checkpoint**: Todo example from spec §5.4 runs; widgets and style stack complete; canvas available for custom drawing.

---

## Phase 7: User Story 5 — Rendering & Fonts (Priority: P5)

**Goal**: Draw command buffer fully consumed by RDI; default font (SDF atlas); default theme (colors, radii).

**Independent Test**: Text renders with default font; buttons/labels use default theme; no placeholder blocks.

- [x] T035 [US5] Implement full draw command submission in src/core/render.c (or equivalent): core produces command buffer; call RDI submit; software RDI and future GPU drivers consume same buffer
- [x] T036 [US5] Add SDF font atlas build (e.g. Noto Sans) and load in src/font/atlas.c; expose default font id to layout/draw; document optional HarfBuzz in build
- [x] T037 [US5] Define default theme (colors, corner radii, font size) in src/widget/theme.c or src/core/theme.h; apply in widget layer when no style override
- [x] T038 [US5] Wire text glyphs to draw commands (position, atlas uv) and RDI text draw in src/rdi/clearui_rdi_soft.c (and future GPU RDI)
- [x] T038b [US5] Implement High-DPI: platform adapter queries display scale factor (DPI) at startup and on display change; scale logical-to-physical coordinates in render path (src/core/render.c or RDI submit); document in include/clearui.h that API uses logical pixels per spec §3.3

**Checkpoint**: Default font and theme applied; rendering pipeline end-to-end; High-DPI scaling in place.

---

## Phase 8: User Story 6 — Accessibility & Polish (Priority: P6)

**Goal**: Accessibility tree from retained nodes; keyboard navigation (Tab, Enter, arrows); WCAG 2.1 AA focus indicators; optional dev overlay.

**Independent Test**: Tab cycles focus; Enter/Space activate; focus ring visible; screen reader sees roles/labels (platform mapping deferred if needed).

- [x] T039 [US6] Build accessibility tree from retained/declared tree in src/core/a11y.c: role, label, state, bounds; expose to platform layer for UI Automation / NSAccessibility / AT-SPI2
- [x] T040 [US6] Implement keyboard focus: Tab/Shift-Tab cycle, Enter/Space activate focused widget; focus order from declaration order; cui_tab_index override in include/clearui.h
- [x] T041 [US6] Add visible focus indicators (outline or ring) meeting WCAG 2.1 AA contrast in default theme; draw in render path for focused node
- [x] T042 [US6] Add cui_aria_label and optional cui_dev_overlay (frame time, node count, arena usage) behind CUI_DEBUG; implement in src/core/context.c or src/widget/overlay.c

**Checkpoint**: Accessibility and keyboard nav; dev overlay optional; ready for polish.

---

## Phase 9: Polish & Cross-Cutting Concerns

**Purpose**: Documentation, quickstart validation, and cleanup.

- [x] T043 [P] Update README or docs with build instructions and link to specs/001-clearui-core/quickstart.md
- [x] T044 Validate quickstart.md: Hello World and Counter build and run with single-command cc from repo root
- [x] T045 Code cleanup: ensure all public API in include/clearui.h under ~120 functions; remove dead code; consistent naming (cui_ prefix, verb_noun)
- [x] T046 [P] Add integration test for RDI + platform (SDL3 + soft RDI) in tests/integration/test_rdi_platform.c if not covered by T019
- [x] T047 Add CI for Tier 1 platforms: configure CI (e.g. GitHub Actions) to build and run unit + integration tests on Windows, macOS, and Linux on every commit per constitution Development Workflow
- [x] T048 [P] Produce two-file distribution option: script or build target to generate amalgamated clearui.c from src/ for primary distribution per spec §7.1, or document in README that amalgamation is a follow-up

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately.
- **Phase 2 (Foundational)**: Depends on Phase 1 — BLOCKS all user stories.
- **Phase 3 (US1)**: Depends on Phase 2 — MVP zero-to-window.
- **Phase 4 (US2)**: Depends on Phase 3 — memory and counter.
- **Phase 5 (US3)**: Depends on Phase 3 — layout can start after US1; may overlap with US2.
- **Phase 6 (US4)**: Depends on Phase 5 — widgets need layout.
- **Phase 7 (US5)**: Depends on Phase 4 and Phase 6 — rendering and fonts.
- **Phase 8 (US6)**: Depends on Phase 6 — a11y needs widget tree.
- **Phase 9 (Polish)**: Depends on Phase 8 (or earlier if US6 deferred). T047 (CI) and T048 (amalgamation) are part of Polish.

### User Story Dependencies

- **US1**: After Foundational only — first runnable demo.
- **US2**: After US1 — uses context and frame; adds vault/button.
- **US3**: After US1 — layout engine and containers.
- **US4**: After US3 — widgets use layout.
- **US5**: After US2/US4 — full draw path and theme.
- **US6**: After US4 — a11y uses widget tree.

### Parallel Opportunities

- T003, T010, T011 can run in parallel with other Phase 1/2 tasks.
- T024 and T028 (layout struct and unit test) can run in parallel within Phase 5.
- T043 and T046 can run in parallel in Phase 9.

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup  
2. Complete Phase 2: Foundational  
3. Complete Phase 3: User Story 1  
4. **STOP and VALIDATE**: Run Hello World; single-command build and run.  
5. Demo or hand off to next story.

### Incremental Delivery

1. Setup + Foundational → foundation ready.  
2. US1 → Hello World runnable (MVP).  
3. US2 → Counter runnable.  
4. US3 → Layout (row/column/center).  
5. US4 → Todo example runnable.  
6. US5 → Fonts and theme.  
7. US6 → Accessibility and polish.  
8. Phase 9 → Docs and quickstart validation.

### Parallel Team Strategy

- After Phase 2: one developer can do US1 (zero-to-window), another can start US3 (layout) in parallel once node types allow.  
- US4 and US5 can overlap (widgets vs. theme/fonts).  
- US6 can start after US4 checkpoint.

---

## Notes

- [P] tasks = different files, no cross-task dependencies.  
- [USn] maps to Phase 3–8 for traceability.  
- Each phase has a checkpoint; stop and validate before proceeding.  
- Tests: unit tests for arena, vault, layout (T010, T011, T028); integration for hello and RDI+platform (T019, T046). CI (T047) runs these on Tier 1.  
- New remediation tasks: tree diff (T013b), cui_run (T023b), cui_stack/cui_wrap (T027b, T027c), canvas (T034b), High-DPI (T038b), CI (T047), amalgamation (T048).  
- File paths follow plan.md: include/, src/core|layout|widget|rdi|platform|font/, tests/unit|integration/.
