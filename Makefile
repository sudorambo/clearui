# ClearUI - C11 build (plan.md: -std=c11 -Wall -Wextra -Wpedantic)
CC     ?= cc
AR     ?= ar
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -I. -Iinclude -Isrc -Isrc/core
LDFLAGS := -lm

# Sources (expand as implementation grows)
CORE_SRCS := src/core/arena.c src/core/frame_alloc.c src/core/vault.c src/core/context.c \
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
test_layout: tests/unit/test_layout.c src/core/arena.c src/core/node.c src/layout/layout.c src/font/atlas.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_layout.c src/core/arena.c src/core/node.c src/layout/layout.c src/font/atlas.c $(LDFLAGS)
test_font: tests/unit/test_font.c src/font/atlas.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_font.c src/font/atlas.c $(LDFLAGS)
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
unit-tests: test_arena test_vault test_layout test_font test_draw_buf test_diff test_frame_alloc test_draw_cmd test_a11y test_focus
	./test_arena && ./test_vault && ./test_layout && ./test_font && ./test_draw_buf && ./test_diff && ./test_frame_alloc && ./test_draw_cmd && ./test_a11y && ./test_focus

# Integration tests
test_hello: $(OBJS) tests/integration/test_hello.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_hello.c $(OBJS) $(LDFLAGS)
test_counter: $(OBJS) tests/integration/test_counter.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_counter.c $(OBJS) $(LDFLAGS)
test_rdi_platform: $(OBJS) tests/integration/test_rdi_platform.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_rdi_platform.c $(OBJS) $(LDFLAGS)
integration-tests: test_hello test_counter test_rdi_platform
	./test_hello && ./test_counter && ./test_rdi_platform

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) build/*.o build/*.a libclearui.a hello counter \
		test_arena test_vault test_layout test_font test_draw_buf test_diff \
		test_frame_alloc test_draw_cmd test_a11y test_focus \
		test_hello test_counter test_rdi_platform 2>/dev/null; true

.PHONY: all lib clean unit-tests integration-tests
