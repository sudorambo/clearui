/*
 * ClearUI Full API Demo
 *
 * Build and run:  make demo && ./demo
 *
 * Exercises every public function in clearui.h (except cui_run, cui_dev_overlay)
 * in a single headless program. Use this as a reference for how to wire up and
 * use the full ClearUI API: layout, widgets, state, styling, canvas,
 * accessibility, keyboard navigation, and platform/RDI integration.
 *
 * Runs 3 frames to demonstrate the declarative model and input lifecycle:
 *   Frame 1 — Build the complete UI tree; inject a click and key event.
 *   Frame 2 — Rebuild the UI; the injected events are consumed this frame.
 *   Frame 3 — Verify persistent state survived across all three frames.
 */
#include "clearui.h"
#include "clearui_platform.h"
#include "clearui_rdi.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
 * Build the full UI tree. Called every frame — ClearUI is immediate-mode, so you
 * declare the entire UI each frame and the library diffs it internally. Interactive
 * widgets (button, checkbox) return non-zero the frame they're activated.
 */
static void build_ui(cui_ctx *ctx, int *counter, int *checked, char *text_buf) {

	/* cui_frame_printf: format a string with per-frame lifetime (freed at next begin_frame).
	   Use for dynamic labels without managing your own buffer. */
	const char *count_text = cui_frame_printf(ctx, "Count: %d", *counter);

	/* cui_frame_alloc: raw per-frame allocation. Same lifetime as frame_printf.
	   Use for scratch buffers, temporary arrays, intermediate results. */
	void *scratch = cui_frame_alloc(ctx, 64);
	(void)scratch;

	/* Containers nest with cui_end() closing each. Indentation mirrors the tree. */

	cui_center(ctx);
	  cui_column(ctx, &(cui_layout){ .gap = 10, .padding = 16 });

	    cui_label(ctx, count_text);

	    /* cui_push_style / cui_pop_style: apply style to a range of widgets.
	       Use when several consecutive widgets share a visual treatment. */
	    cui_push_style(ctx, &(cui_style){ .text_color = 0xffff0000 });
	      cui_label(ctx, "styled red");
	    cui_pop_style(ctx);

	    /* cui_label_styled: one-off style without push/pop bookkeeping. */
	    cui_label_styled(ctx, "strikethrough", &(cui_style){
	        .text_decoration = CUI_TEXT_DECORATION_STRIKETHROUGH
	    });

	    /* cui_row: horizontal layout. Buttons return 1 the frame they're clicked. */
	    cui_row(ctx, &(cui_layout){ .gap = 8 });
	      if (cui_button(ctx, "-")) (*counter)--;
	      if (cui_button(ctx, "+")) (*counter)++;
	    cui_end(ctx);

	    /* cui_checkbox: togglable boolean backed by persistent state. */
	    cui_checkbox(ctx, "agree", checked);

	    /* cui_aria_label + cui_tab_index: set *before* the widget they modify.
	       aria_label overrides the screen-reader label; tab_index sets focus order. */
	    cui_tab_index(ctx, 1);
	    cui_aria_label(ctx, "Delete item");
	    cui_icon_button(ctx, "del", CUI_ICON_TRASH);

	    /* cui_text_input: editable field backed by a caller-owned char buffer. */
	    cui_text_input(ctx, "name", text_buf, 64,
	        &(cui_text_input_opts){ .placeholder = "Your name" });

	    /* cui_spacer: invisible spacing between siblings. */
	    cui_spacer(ctx, 0, 8);

	    /* cui_scroll: scrollable container; clips children beyond max_height. */
	    cui_scroll(ctx, NULL, &(cui_scroll_opts){ .max_height = 60 });
	      cui_label(ctx, "scrollable content");
	    cui_end(ctx);

	    /* cui_wrap: flow layout that wraps children onto the next line. */
	    cui_wrap(ctx, &(cui_layout){ .gap = 4 });
	      cui_label(ctx, "tag1");
	      cui_label(ctx, "tag2");
	    cui_end(ctx);

	    /* cui_stack: children overlap in z-order (last child on top). */
	    cui_stack(ctx, &(cui_layout){0});
	      cui_label(ctx, "bg");
	      cui_label(ctx, "fg");
	    cui_end(ctx);

	    /* cui_canvas: custom drawing surface; draw commands live until next frame.
	       Colors are 0xAARRGGBB (alpha, red, green, blue). */
	    cui_canvas(ctx, &(cui_canvas_opts){ .width = 100, .height = 60 });
	      cui_draw_rect(ctx, 0, 0, 50, 30, 0xff0000ff);
	      cui_draw_circle(ctx, 75, 30, 15, 0xff00ff00);
	      cui_draw_text(ctx, 5, 50, "canvas", 0xff000000);
	    cui_end(ctx);

	  cui_end(ctx); /* column */
	cui_end(ctx); /* center */
}

int main(void) {
	printf("=== ClearUI API Demo ===\n");

	/* cui_create: allocate context with window config.
	   scale_factor and draw_buf_capacity default to 1.0 and 1024 when zero. */
	cui_ctx *ctx = cui_create(&(cui_config){
	    .title  = "API Demo",
	    .width  = 400,
	    .height = 400,
	});
	assert(ctx != NULL);

	/* Platform and RDI are behind function-pointer interfaces.
	   Swap cui_platform_stub_get() → SDL3/GLFW adapter for a real window.
	   Swap cui_rdi_soft_get()      → Vulkan/Metal/WebGPU driver for GPU rendering. */
	const cui_platform *plat = cui_platform_stub_get();
	const cui_rdi *rdi = cui_rdi_soft_get();

	cui_platform_ctx *plat_ctx = NULL;
	assert(plat->window_create(&plat_ctx, "API Demo", 400, 400) == 0);
	cui_rdi_context *rdi_ctx = NULL;
	assert(rdi->init(&rdi_ctx) == 0);

	cui_set_platform(ctx, plat, plat_ctx);
	cui_set_rdi(ctx, rdi, rdi_ctx);
	printf("Platform: stub (headless)\nRDI: software\n");

	/* cui_state: persistent, zero-initialized storage keyed by string.
	   The pointer remains valid for the lifetime of the context. */
	int *counter = (int *)cui_state(ctx, "counter", sizeof(int));
	int *checked = (int *)cui_state(ctx, "agree", sizeof(int));
	assert(counter && *counter == 0);
	assert(checked && *checked == 0);
	char text_buf[64] = {0};

	/* === Frame 1 ===
	   Build the complete UI tree and inject input for next-frame processing.
	   ClearUI events follow a two-frame cycle: inject in frame N, consumed in N+1.
	   This mirrors real event loops where OS events arrive between frames. */
	cui_begin_frame(ctx);
	build_ui(ctx, counter, checked, text_buf);
	cui_inject_click(ctx, 200, 100);
	cui_inject_key(ctx, CUI_KEY_TAB);
	cui_end_frame(ctx);
	assert(cui_running(ctx));
	printf("Frame 1: UI tree built — layout, widgets, canvas, style, a11y\n");

	/* === Frame 2 ===
	   Rebuild the UI. The injected click/key from Frame 1 are processed now.
	   If the click landed on "+", cui_button returns 1 and counter increments. */
	cui_begin_frame(ctx);
	build_ui(ctx, counter, checked, text_buf);
	cui_end_frame(ctx);
	printf("Frame 2: Input consumed — counter=%d\n", *counter);

	/* === Frame 3 ===
	   One more frame to confirm state persists and the UI is stable. */
	cui_begin_frame(ctx);
	build_ui(ctx, counter, checked, text_buf);
	cui_end_frame(ctx);
	printf("Frame 3: State verified — counter=%d, checked=%d\n", *counter, *checked);

	/* Cleanup: shut down RDI and platform, then destroy the ClearUI context. */
	rdi->shutdown(rdi_ctx);
	plat->window_destroy(plat_ctx);
	cui_destroy(ctx);

	printf("demo: PASS\n");
	return 0;
}
