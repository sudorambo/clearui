# Tasks: Full API Demo Example

**Input**: Design documents from `/specs/013-api-demo-example/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to

---

## Phase 1: Setup

**Purpose**: Create the examples directory and verify build baseline.

- [x] T001 Verify clean build baseline: `make clean && make all && make unit-tests && make integration-tests`
- [x] T002 [P] Create `examples/` directory

**Checkpoint**: Existing project builds clean, directory ready.

---

## Phase 2: US1 — Write `examples/demo.c` (Priority: P1) MVP

**Goal**: A single, comprehensive, commented C file that exercises all 34 public API functions (except `cui_run`, `cui_dev_overlay`) across 3 frames.

**Independent Test**: `make demo && ./demo` exits 0 with summary output.

### Implementation

- [x] T003 [US1] Write file header comment in `examples/demo.c` — purpose, build command (`make demo && ./demo`), API coverage summary
- [x] T004 [US1] Write `main()` setup section — includes, `cui_create` with `cui_config`, platform/RDI wiring via `cui_set_platform`/`cui_set_rdi` using stubs, with comments noting swap points for SDL3/Vulkan
- [x] T005 [US1] Write Frame 1 — full UI tree build:
  - Persistent state: `cui_state` (counter int, checkbox int)
  - Frame-scoped allocation: `cui_frame_alloc`, `cui_frame_printf`
  - Layout nesting: `cui_center` > `cui_column` > `cui_row`, plus `cui_scroll`, `cui_wrap`, `cui_stack`, each closed with `cui_end`
  - All widgets: `cui_label`, `cui_label_styled`, `cui_button` x2 ("+", "-"), `cui_checkbox`, `cui_icon_button`, `cui_text_input`, `cui_spacer`
  - Styling: `cui_push_style` / `cui_pop_style` wrapping a styled label
  - Canvas: `cui_canvas` > `cui_draw_rect`, `cui_draw_circle`, `cui_draw_text`, closed with `cui_end`
  - Accessibility: `cui_aria_label` + `cui_tab_index` before a widget
  - Assert `cui_running(ctx)` returns true after frame
  - Print Frame 1 summary
- [x] T006 [US1] Write Frame 2 — input injection:
  - `cui_inject_click` targeting the "+" button position
  - `cui_inject_key(CUI_KEY_TAB)` for focus movement
  - Rebuild full UI tree (same structure as Frame 1)
  - Capture `cui_button` return value for "+" to confirm click consumed
  - Increment counter on click
  - Print Frame 2 summary
- [x] T007 [US1] Write Frame 3 — state verification:
  - Rebuild UI tree
  - Print Frame 3 summary with final state values
- [x] T008 [US1] Write cleanup section — `rdi->shutdown`, `plat->window_destroy`, `cui_destroy`, print `"demo: PASS\n"`
- [x] T009 [US1] Review full file: verify all 34 API functions are called, comments explain *why* not *what*, file is under 200 lines, zero `#ifdef`

**Checkpoint**: `examples/demo.c` is complete and correct. Not yet buildable (no Makefile target).

---

## Phase 3: US2 — Build Integration (Priority: P1)

**Goal**: `make demo` compiles and links `examples/demo.c` against the library objects.

**Independent Test**: `make demo && ./demo` exits 0.

### Implementation

- [x] T010 [US2] Add `demo` target to `Makefile`: `$(CC) $(CFLAGS) -o $@ examples/demo.c $(OBJS) $(LDFLAGS)` with `$(OBJS)` dependency
- [x] T011 [US2] Add `demo` to `clean` rule in `Makefile`
- [x] T012 [US2] Add `demo` to `.PHONY` in `Makefile`
- [x] T013 [US2] Verify: `make demo && ./demo` exits 0, zero warnings

**Checkpoint**: Demo builds, runs, prints summary, exits 0.

---

## Phase 4: US3 — README Integration (Priority: P2)

**Goal**: Developers discover the demo from the README.

### Implementation

- [x] T014 [P] [US3] Add "Examples" section to `README.md` between "Build" and "Project Structure" sections — link to `examples/demo.c`, `make demo && ./demo` command, brief description of what it covers

**Checkpoint**: README links to the example.

---

## Phase 5: Polish & Validation

**Purpose**: Full pipeline validation and acceptance criteria check.

- [x] T015 Full pipeline: `make clean && make all && make unit-tests && make integration-tests && make demo && ./demo`
- [x] T016 Verify zero warnings under `-std=c11 -Wall -Wextra -Wpedantic`
- [x] T017 Verify all acceptance criteria from spec.md:
  - `make demo && ./demo` exits 0
  - All 34 API functions called (except `cui_run`, `cui_dev_overlay`)
  - Comments explain purpose of each section
  - README links to example
  - Zero warnings

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately
- **Phase 2 (US1: demo.c)**: Depends on Phase 1 (T002 for directory)
- **Phase 3 (US2: Makefile)**: Depends on Phase 2 (needs demo.c to exist)
- **Phase 4 (US3: README)**: Can run in parallel with Phase 3
- **Phase 5 (Validation)**: Depends on all prior phases

### Parallel Opportunities

- T001 and T002 can run in parallel (different concerns)
- T003–T009 are sequential within the file (logical sections build on each other)
- T010–T012 are sequential within Makefile (same file)
- T014 (README) can run in parallel with Phase 3 (different file)

### Task Count

| Phase | Tasks | Parallel |
|-------|-------|----------|
| Setup | 2 | 2 |
| US1: demo.c | 7 | 0 (sequential) |
| US2: Makefile | 4 | 0 (same file) |
| US3: README | 1 | 1 |
| Validation | 3 | 0 (sequential) |
| **Total** | **17** | |
