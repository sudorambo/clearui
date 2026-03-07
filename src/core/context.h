#ifndef CLEARUI_CONTEXT_H
#define CLEARUI_CONTEXT_H

#include "../../include/clearui.h"
#include "../../include/clearui_rdi.h"
#include "arena.h"
#include "frame_alloc.h"
#include "vault.h"
#include "draw_cmd.h"
#include "node.h"
#include "a11y.h"

cui_ctx *cui_create(const cui_config *config);
void     cui_destroy(cui_ctx *ctx);

void cui_begin_frame(cui_ctx *ctx);
void cui_end_frame(cui_ctx *ctx);
int  cui_running(cui_ctx *ctx);

void cui_set_platform(cui_ctx *ctx, const cui_platform *platform, cui_platform_ctx *platform_ctx);
void cui_set_rdi(cui_ctx *ctx, const cui_rdi *rdi, cui_rdi_context *rdi_ctx);

cui_arena       *cui_ctx_arena(cui_ctx *ctx);
cui_frame_allocator *cui_ctx_frame(cui_ctx *ctx);
cui_vault       *cui_ctx_vault(cui_ctx *ctx);
cui_draw_command_buffer *cui_ctx_draw_buf(cui_ctx *ctx);
const cui_config *cui_ctx_config(cui_ctx *ctx);
const cui_rdi   *cui_ctx_rdi(cui_ctx *ctx);
cui_rdi_context *cui_ctx_rdi_ctx(cui_ctx *ctx);
cui_node       **cui_ctx_root_ptr(cui_ctx *ctx);
cui_node        *cui_ctx_current_parent(cui_ctx *ctx);
void             cui_ctx_push_parent(cui_ctx *ctx, cui_node *n);
void             cui_ctx_pop_parent(cui_ctx *ctx);

int              cui_ctx_consume_click(cui_ctx *ctx, const char *id);

const cui_style  *cui_ctx_current_style(cui_ctx *ctx);

void             cui_ctx_set_canvas_node(cui_ctx *ctx, cui_node *n);
cui_node        *cui_ctx_canvas_node(cui_ctx *ctx);
cui_draw_command_buffer *cui_ctx_canvas_buf(cui_ctx *ctx);
cui_draw_command_buffer *cui_ctx_scaled_buf(cui_ctx *ctx);
cui_draw_command_buffer *cui_ctx_current_draw_buf(cui_ctx *ctx);

const char       *cui_ctx_focused_id(cui_ctx *ctx);
const char       *cui_ctx_take_aria_label(cui_ctx *ctx);
int               cui_ctx_take_tab_index(cui_ctx *ctx);

cui_node        *cui_ctx_declared_root(cui_ctx *ctx);
void             cui_ctx_build_a11y(cui_ctx *ctx);
const cui_a11y_tree *cui_ctx_a11y_tree(cui_ctx *ctx);

#endif
