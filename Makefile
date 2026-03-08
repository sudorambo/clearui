# ClearUI - C11 build (plan.md: -std=c11 -Wall -Wextra -Wpedantic)
CC     ?= cc
AR     ?= ar
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -I. -Iinclude -Isrc -Isrc/core
LDFLAGS := -lm

# Optional: WITH_SDL3=1 builds SDL3 platform adapter and test_platform_window (requires SDL3)
WITH_SDL3 ?= 0
SDL3_CFLAGS := $(shell sdl3-config --cflags 2>/dev/null || pkg-config --cflags SDL3 2>/dev/null)
SDL3_LDFLAGS := $(shell sdl3-config --libs 2>/dev/null || pkg-config --libs SDL3 2>/dev/null)
# Fallback if no sdl3-config/pkg-config: assume -lSDL3 and default include path
ifeq ($(SDL3_LDFLAGS),)
SDL3_LDFLAGS := -lSDL3
endif

# Sources (expand as implementation grows)
CORE_SRCS := src/core/arena.c src/core/frame_alloc.c src/core/vault.c src/core/utf8.c src/core/context.c \
	src/core/draw_cmd.c src/core/node.c src/core/diff.c src/core/render.c src/core/a11y.c
FONT_SRCS := src/font/atlas.c
LAYOUT_SRCS := src/layout/layout.c
WIDGET_SRCS := src/widget/layout.c src/widget/label.c src/widget/button.c src/widget/checkbox.c \
	src/widget/spacer.c src/widget/icon_button.c src/widget/scroll.c src/widget/text_input.c src/widget/canvas.c
RDI_SRCS   := src/rdi/clearui_rdi_soft.c
PLAT_SRCS  := src/platform/cui_platform_stub.c
SRCS       := $(CORE_SRCS) $(FONT_SRCS) $(LAYOUT_SRCS) $(WIDGET_SRCS) $(RDI_SRCS) $(PLAT_SRCS)
OBJS       := $(SRCS:.c=.o)

LIB_SRCS   := $(CORE_SRCS) $(FONT_SRCS) $(LAYOUT_SRCS) $(WIDGET_SRCS)
LIB_OBJS   := $(LIB_SRCS:.c=.o)

# Default: build all objects
all: $(OBJS)

# Static library (core + font + layout + widgets; excludes platform/RDI stubs)
lib: libclearui.a
libclearui.a: $(LIB_OBJS)
	$(AR) rcs $@ $(LIB_OBJS)

# Unit tests
test_arena: tests/unit/test_arena.c src/core/arena.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_arena.c src/core/arena.c $(LDFLAGS)
test_vault: tests/unit/test_vault.c src/core/vault.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_vault.c src/core/vault.c $(LDFLAGS)
test_layout: tests/unit/test_layout.c src/core/arena.o src/core/frame_alloc.o src/core/vault.o src/core/utf8.o src/core/node.o src/core/diff.o src/core/draw_cmd.o src/core/render.o src/core/a11y.o src/core/context.o src/layout/layout.o src/font/atlas.o
	$(CC) $(CFLAGS) -o $@ tests/unit/test_layout.c src/core/arena.o src/core/frame_alloc.o src/core/vault.o src/core/utf8.o src/core/node.o src/core/diff.o src/core/draw_cmd.o src/core/render.o src/core/a11y.o src/core/context.o src/layout/layout.o src/font/atlas.o $(LDFLAGS)
test_font: tests/unit/test_font.c src/font/atlas.c src/core/utf8.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_font.c src/font/atlas.c src/core/utf8.c $(LDFLAGS)
test_draw_buf: $(OBJS) tests/unit/test_draw_buf.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_draw_buf.c $(OBJS) $(LDFLAGS)
test_diff: tests/unit/test_diff.c src/core/arena.o src/core/node.o src/core/diff.o
	$(CC) $(CFLAGS) -o $@ tests/unit/test_diff.c src/core/arena.o src/core/node.o src/core/diff.o $(LDFLAGS)
test_frame_alloc: tests/unit/test_frame_alloc.c src/core/frame_alloc.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_frame_alloc.c src/core/frame_alloc.c $(LDFLAGS)
test_draw_cmd: $(OBJS) tests/unit/test_draw_cmd.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_draw_cmd.c $(OBJS) $(LDFLAGS)
test_a11y: $(OBJS) tests/unit/test_a11y.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_a11y.c $(OBJS) $(LDFLAGS)
test_focus: $(OBJS) tests/unit/test_focus.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_focus.c $(OBJS) $(LDFLAGS)
test_text_input: $(OBJS) tests/unit/test_text_input.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_text_input.c $(OBJS) $(LDFLAGS)
test_scroll: $(OBJS) tests/unit/test_scroll.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_scroll.c $(OBJS) $(LDFLAGS)
test_canvas_draw: $(OBJS) tests/unit/test_canvas_draw.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_canvas_draw.c $(OBJS) $(LDFLAGS)
test_label_styled: $(OBJS) tests/unit/test_label_styled.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_label_styled.c $(OBJS) $(LDFLAGS)
test_spacer: $(OBJS) tests/unit/test_spacer.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_spacer.c $(OBJS) $(LDFLAGS)
test_wrap: $(OBJS) tests/unit/test_wrap.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_wrap.c $(OBJS) $(LDFLAGS)
test_stack: $(OBJS) tests/unit/test_stack.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_stack.c $(OBJS) $(LDFLAGS)
test_style_stack: $(OBJS) tests/unit/test_style_stack.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_style_stack.c $(OBJS) $(LDFLAGS)
test_cui_frame_alloc: $(OBJS) tests/unit/test_cui_frame_alloc.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_cui_frame_alloc.c $(OBJS) $(LDFLAGS)
test_scale_buf: $(OBJS) tests/unit/test_scale_buf.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_scale_buf.c $(OBJS) $(LDFLAGS)
test_edge_cases: $(OBJS) tests/unit/test_edge_cases.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_edge_cases.c $(OBJS) $(LDFLAGS)
test_theme: $(OBJS) tests/unit/test_theme.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_theme.c $(OBJS) $(LDFLAGS)
test_rdi_soft: $(OBJS) tests/unit/test_rdi_soft.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_rdi_soft.c $(OBJS) $(LDFLAGS)
test_utf8: tests/unit/test_utf8.c src/core/utf8.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_utf8.c src/core/utf8.c $(LDFLAGS)
unit-tests: test_arena test_vault test_layout test_font test_draw_buf test_diff test_frame_alloc test_draw_cmd test_a11y test_focus test_text_input test_scroll test_canvas_draw test_label_styled test_spacer test_wrap test_stack test_style_stack test_cui_frame_alloc test_scale_buf test_edge_cases test_theme test_rdi_soft test_utf8
	./test_arena && ./test_vault && ./test_layout && ./test_font && ./test_draw_buf && ./test_diff && ./test_frame_alloc && ./test_draw_cmd && ./test_a11y && ./test_focus && ./test_text_input && ./test_scroll && ./test_canvas_draw && ./test_label_styled && ./test_spacer && ./test_wrap && ./test_stack && ./test_style_stack && ./test_cui_frame_alloc && ./test_scale_buf && ./test_edge_cases && ./test_theme && ./test_rdi_soft && ./test_utf8

# Integration tests
test_hello: $(OBJS) tests/integration/test_hello.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_hello.c $(OBJS) $(LDFLAGS)
test_counter: $(OBJS) tests/integration/test_counter.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_counter.c $(OBJS) $(LDFLAGS)
test_rdi_platform: $(OBJS) tests/integration/test_rdi_platform.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_rdi_platform.c $(OBJS) $(LDFLAGS)
test_text_input_edit: $(OBJS) tests/integration/test_text_input_edit.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_text_input_edit.c $(OBJS) $(LDFLAGS)
test_scroll_region: $(OBJS) tests/integration/test_scroll_region.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_scroll_region.c $(OBJS) $(LDFLAGS)
# SDL3: real window, one frame, close (build only when WITH_SDL3=1)
ifeq ($(WITH_SDL3),1)
src/platform/cui_platform_sdl3.o: src/platform/cui_platform_sdl3.c
	$(CC) $(CFLAGS) $(SDL3_CFLAGS) -c -o $@ $<
test_platform_window: $(OBJS) src/platform/cui_platform_sdl3.o tests/integration/test_platform_window.c
	$(CC) $(CFLAGS) $(SDL3_CFLAGS) -o $@ tests/integration/test_platform_window.c $(OBJS) src/platform/cui_platform_sdl3.o $(LDFLAGS) $(SDL3_LDFLAGS)
integration-tests: test_hello test_counter test_rdi_platform test_text_input_edit test_scroll_region test_platform_window
	./test_hello && ./test_counter && ./test_rdi_platform && ./test_text_input_edit && ./test_scroll_region && ./test_platform_window
else
integration-tests: test_hello test_counter test_rdi_platform test_text_input_edit test_scroll_region
	./test_hello && ./test_counter && ./test_rdi_platform && ./test_text_input_edit && ./test_scroll_region
endif

# Examples
demo: $(OBJS) examples/demo.c
	$(CC) $(CFLAGS) -o $@ examples/demo.c $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) src/platform/cui_platform_sdl3.o build/*.o build/*.a libclearui.a hello counter demo \
		test_arena test_vault test_layout test_font test_draw_buf test_diff \
		test_frame_alloc test_draw_cmd test_a11y test_focus test_text_input test_scroll \
		test_canvas_draw test_label_styled test_spacer test_wrap test_stack test_style_stack test_cui_frame_alloc test_scale_buf test_edge_cases test_theme test_rdi_soft test_utf8 \
		fuzz_utf8 fuzz_vault fuzz_frame stress \
		test_hello test_counter test_rdi_platform test_text_input_edit test_scroll_region test_platform_window 2>/dev/null; true

# Sanitizer builds (recompile everything with sanitizer flags)
ASAN_FLAGS := -fsanitize=address -fno-omit-frame-pointer -g
UBSAN_FLAGS := -fsanitize=undefined -fno-omit-frame-pointer -g

asan: clean
	$(MAKE) all unit-tests integration-tests CFLAGS="$(CFLAGS) $(ASAN_FLAGS)" LDFLAGS="$(LDFLAGS) $(ASAN_FLAGS)"

ubsan: clean
	$(MAKE) all unit-tests integration-tests CFLAGS="$(CFLAGS) $(UBSAN_FLAGS)" LDFLAGS="$(LDFLAGS) $(UBSAN_FLAGS)"

# Fuzz (libFuzzer): build with -fsanitize=fuzzer; run with ./fuzz_* [corpus] or -runs=N
fuzz-utf8: tests/fuzz/utf8_fuzz.c src/core/utf8.c
	$(CC) $(CFLAGS) -fsanitize=fuzzer -o fuzz_utf8 tests/fuzz/utf8_fuzz.c src/core/utf8.c $(LDFLAGS) -fsanitize=fuzzer
fuzz-vault: tests/fuzz/vault_fuzz.c src/core/vault.c
	$(CC) $(CFLAGS) -DNO_LIBFUZZER -o fuzz_vault tests/fuzz/vault_fuzz.c src/core/vault.c $(LDFLAGS)
fuzz-frame: tests/fuzz/frame_fuzz.c src/core/frame_alloc.c
	$(CC) $(CFLAGS) -DNO_LIBFUZZER -o fuzz_frame tests/fuzz/frame_fuzz.c src/core/frame_alloc.c $(LDFLAGS)

# Stress: 1000+ widgets, 10 frames
stress: $(OBJS) tests/stress/stress_widgets.c
	$(CC) $(CFLAGS) -o stress tests/stress/stress_widgets.c $(OBJS) $(LDFLAGS)
	./stress

# Leak check: run unit tests under Valgrind (Linux). Requires valgrind.
# If Valgrind fails at startup (e.g. "memcmp redirection" in ld.so), use: make leak-check-lsan
# Only "definitely lost" counts as failure (--errors-for-leak-kinds=definite); "still reachable" (e.g. libc) is ignored.
leak-check: unit-tests
	@echo "Running unit tests under Valgrind (leak-check=full, errors only for definitely lost)..."
	@for t in ./test_arena ./test_vault ./test_layout ./test_font ./test_draw_buf ./test_diff ./test_frame_alloc ./test_draw_cmd ./test_a11y ./test_focus ./test_text_input ./test_scroll ./test_canvas_draw ./test_label_styled ./test_spacer ./test_wrap ./test_stack ./test_style_stack ./test_cui_frame_alloc ./test_scale_buf ./test_edge_cases ./test_theme ./test_rdi_soft ./test_utf8; do \
	  valgrind --leak-check=full --errors-for-leak-kinds=definite --error-exitcode=1 --quiet $$t 2>/dev/null || { echo "Leak in $$t"; exit 1; }; \
	done
	@echo "Leak check passed."

# Leak check via LeakSanitizer (no Valgrind). Use when Valgrind fails at startup
# (e.g. "memcmp redirection" / ld.so on some glibc). Rebuilds tests with -fsanitize=leak.
leak-check-lsan: unit-tests
	@echo "Running unit tests with LeakSanitizer (-fsanitize=leak)..."
	$(MAKE) -s unit-tests CFLAGS="$(CFLAGS) -fsanitize=leak -g" LDFLAGS="$(LDFLAGS) -fsanitize=leak"
	@echo "LeakSanitizer run passed (no leaks reported)."

.PHONY: all lib clean unit-tests integration-tests demo asan ubsan fuzz-utf8 fuzz-vault fuzz-frame stress leak-check leak-check-lsan
