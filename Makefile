# ClearUI - C11 build (plan.md: -std=c11 -Wall -Wextra -Wpedantic)
CC     ?= cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -I. -Iinclude -Isrc
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

# Default: build all objects
all: $(OBJS)

# Unit tests (Phase 2)
test_arena: tests/unit/test_arena.c src/core/arena.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_arena.c src/core/arena.c $(LDFLAGS)
test_vault: tests/unit/test_vault.c src/core/vault.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_vault.c src/core/vault.c $(LDFLAGS)
test_layout: tests/unit/test_layout.c src/core/arena.c src/core/node.c src/layout/layout.c src/font/atlas.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_layout.c src/core/arena.c src/core/node.c src/layout/layout.c src/font/atlas.c $(LDFLAGS)
test_font: tests/unit/test_font.c src/font/atlas.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_font.c src/font/atlas.c $(LDFLAGS)
unit-tests: test_arena test_vault test_layout test_font
	./test_arena && ./test_vault && ./test_layout && ./test_font

# Integration test: Hello World pipeline (Phase 3)
test_hello: $(OBJS) tests/integration/test_hello.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_hello.c $(OBJS) $(LDFLAGS)
# Integration test: Counter (Phase 4)
test_counter: $(OBJS) tests/integration/test_counter.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_counter.c $(OBJS) $(LDFLAGS)
# Integration test: RDI + platform (Phase 9)
test_rdi_platform: $(OBJS) tests/integration/test_rdi_platform.c
	$(CC) $(CFLAGS) -o $@ tests/integration/test_rdi_platform.c $(OBJS) $(LDFLAGS)
integration-tests: test_hello test_counter test_rdi_platform
	./test_hello && ./test_counter && ./test_rdi_platform

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) build/*.o build/*.a hello counter test_arena test_vault test_layout test_font test_hello test_counter test_rdi_platform 2>/dev/null; true

.PHONY: all clean unit-tests integration-tests
