# Quickstart: Test Coverage (0.5.0)

**Branch**: `018-test-coverage-0.5.0`

## Running the new tests

After implementation, new test binaries will be added to the Makefile. From the repository root:

```bash
make unit-tests
```

This builds and runs all unit tests, including the new coverage tests (canvas draw, label_styled, spacer, wrap, stack, style stack, frame_alloc, scale_buf, edge cases).

Run with sanitizers (recommended before merging):

```bash
make asan
make ubsan
```

## Test layout

- **tests/unit/** — New files such as `test_canvas_draw.c`, `test_label_styled.c`, `test_spacer.c`, `test_wrap.c`, `test_stack.c`, `test_style_stack.c`, `test_frame_alloc.c`, `test_scale_buf.c`, `test_edge_cases.c`.
- Tests that need to inspect draw buffer or layout include `../../src/core/context.h` and optionally `../../src/core/draw_cmd.h`, `../../src/core/node.h`.
- Each test creates a minimal context (with stub platform/RDI when required), runs one or more frames, and asserts on buffer contents, node fields, or style.

## Adding a new test

1. Create `tests/unit/test_<name>.c` with a `main()` that builds context, calls the API under test, and asserts expected outcome.
2. Add a target in the Makefile, e.g. `test_<name>: $(OBJS) tests/unit/test_<name>.c` and link with `$(OBJS)`.
3. Add `test_<name>` to the `unit-tests` phony target’s dependency list and to the list of executables run in the recipe.
4. Add `test_<name>` to the `clean` target’s `rm -f` list.
5. Ensure `make asan` and `make ubsan` build and run this test (they use the same `unit-tests` / `integration-tests` targets).

## Success criteria

- Every public API listed in the spec (canvas, draw_*, label_styled, spacer, wrap, stack, push_style, pop_style, frame_alloc) is exercised by at least one test.
- Hi-DPI scale_buf path is tested with scale_factor > 1.
- Edge-case tests cover NULL ctx and zero-size/empty inputs where applicable.
- `make unit-tests` and `make integration-tests` pass; `make asan` and `make ubsan` pass with no sanitizer errors.
