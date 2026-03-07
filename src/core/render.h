#ifndef CLEARUI_RENDER_H
#define CLEARUI_RENDER_H

/**
 * Render: submit the context's draw command buffer to the RDI.
 * Applies scale_factor (logical -> physical) when != 1.0.
 * Core produces buffer; software and GPU RDI drivers consume the same buffer format.
 */
struct cui_ctx;
void cui_render_submit(struct cui_ctx *ctx);

#endif
