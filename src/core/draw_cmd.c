#include "draw_cmd.h"
#include "node.h"
#include "core/context.h"
#include "core/theme.h"
#include <stdlib.h>
#include <string.h>

#define DECO_STRIKETHROUGH 1

static void push_focus_ring(cui_draw_command_buffer *buf, float x, float y, float w, float h, float ox, float oy) {
	float t = CUI_THEME_FOCUS_RING_WIDTH;
	float x0 = x + ox, y0 = y + oy;
	cui_draw_buf_push_line(buf, x0, y0, x0 + w, y0, t, CUI_THEME_FOCUS_RING_COLOR);
	cui_draw_buf_push_line(buf, x0 + w, y0, x0 + w, y0 + h, t, CUI_THEME_FOCUS_RING_COLOR);
	cui_draw_buf_push_line(buf, x0 + w, y0 + h, x0, y0 + h, t, CUI_THEME_FOCUS_RING_COLOR);
	cui_draw_buf_push_line(buf, x0, y0 + h, x0, y0, t, CUI_THEME_FOCUS_RING_COLOR);
}

static unsigned int node_text_color(const cui_node *n) {
	return n->text_color ? n->text_color : CUI_THEME_DEFAULT_TEXT_COLOR;
}

static void emit_label(cui_node *root, cui_draw_command_buffer *buf, float ox, float oy) {
	if (!root->label_text) return;
	unsigned int color = node_text_color(root);
	cui_draw_buf_push_text(buf, root->layout_x + ox, root->layout_y + oy, root->label_text, color);
	if (root->text_decoration == DECO_STRIKETHROUGH) {
		float mid_y = root->layout_y + root->layout_h / 2 + oy;
		cui_draw_buf_push_line(buf, root->layout_x + ox, mid_y, root->layout_x + root->layout_w + ox, mid_y, 1.f, color);
	}
}

static void replay_canvas_buf(cui_draw_command_buffer *dst, const cui_draw_command_buffer *src, float ox, float oy) {
	if (!dst || !src) return;
	for (size_t i = 0; i < src->count; i++) {
		const cui_draw_cmd *c = &src->cmd[i];
		switch (c->type) {
		case CUI_CMD_RECT:
			cui_draw_buf_push_rect(dst, c->u.fill_rect.x + ox, c->u.fill_rect.y + oy, c->u.fill_rect.w, c->u.fill_rect.h, c->u.fill_rect.color);
			break;
		case CUI_CMD_TEXT:
			cui_draw_buf_push_text(dst, c->u.text.x + ox, c->u.text.y + oy, c->u.text.text, c->u.text.color);
			break;
		case CUI_CMD_LINE:
			cui_draw_buf_push_line(dst, c->u.line.a.x + ox, c->u.line.a.y + oy, c->u.line.b.x + ox, c->u.line.b.y + oy, c->u.line.thickness, c->u.line.color);
			break;
		default:
			break;
		}
	}
}

void cui_build_draw_from_tree(cui_ctx *ctx, cui_node *root, cui_draw_command_buffer *buf, float offset_x, float offset_y) {
	if (!root || !buf) return;
	float ox = offset_x, oy = offset_y;
	const char *focused_id = ctx ? cui_ctx_focused_id(ctx) : NULL;
	int is_focused = (focused_id && root->button_id && strcmp(root->button_id, focused_id) == 0);

	if (root->type == CUI_NODE_LABEL) emit_label(root, buf, ox, oy);
	if (root->type == CUI_NODE_BUTTON) {
		cui_draw_buf_push_rect(buf, root->layout_x + ox, root->layout_y + oy, root->layout_w, root->layout_h, CUI_THEME_DEFAULT_BUTTON_BG);
		if (root->label_text)
			cui_draw_buf_push_text(buf, root->layout_x + ox + 4, root->layout_y + oy + 4, root->label_text, node_text_color(root));
		if (is_focused) push_focus_ring(buf, root->layout_x, root->layout_y, root->layout_w, root->layout_h, ox, oy);
	}
	if (root->type == CUI_NODE_CHECKBOX) {
		cui_draw_buf_push_rect(buf, root->layout_x + ox, root->layout_y + oy, root->layout_w, root->layout_h, CUI_THEME_DEFAULT_CHECKBOX_BG);
		if (root->label_text)
			cui_draw_buf_push_text(buf, root->layout_x + ox + 2, root->layout_y + oy + 2, root->label_text, node_text_color(root));
		if (is_focused) push_focus_ring(buf, root->layout_x, root->layout_y, root->layout_w, root->layout_h, ox, oy);
	}
	if (root->type == CUI_NODE_ICON_BUTTON) {
		cui_draw_buf_push_rect(buf, root->layout_x + ox, root->layout_y + oy, root->layout_w, root->layout_h, CUI_THEME_DEFAULT_BUTTON_BG);
		if (root->label_text)
			cui_draw_buf_push_text(buf, root->layout_x + ox + 4, root->layout_y + oy + 4, root->label_text, node_text_color(root));
		if (is_focused) push_focus_ring(buf, root->layout_x, root->layout_y, root->layout_w, root->layout_h, ox, oy);
	}
	if (root->type == CUI_NODE_SCROLL) {
		float child_oy = oy + root->layout_y - root->scroll_offset_y;
		for (cui_node *c = root->first_child; c; c = c->next_sibling)
			cui_build_draw_from_tree(ctx, c, buf, ox + root->layout_x, child_oy);
		return;
	}
	if (root->type == CUI_NODE_TEXT_INPUT) {
		cui_draw_buf_push_rect(buf, root->layout_x + ox, root->layout_y + oy, root->layout_w, root->layout_h, CUI_THEME_DEFAULT_INPUT_BG);
		if (root->text_input_buf && root->text_input_cap > 0)
			cui_draw_buf_push_text(buf, root->layout_x + ox + 4, root->layout_y + oy + 4, root->text_input_buf, CUI_THEME_DEFAULT_TEXT_COLOR);
		if (is_focused) push_focus_ring(buf, root->layout_x, root->layout_y, root->layout_w, root->layout_h, ox, oy);
	}
	if (ctx && root->type == CUI_NODE_CANVAS && cui_ctx_canvas_node(ctx) == root) {
		replay_canvas_buf(buf, cui_ctx_canvas_buf(ctx), root->layout_x + ox, root->layout_y + oy);
		return;
	}
	for (cui_node *c = root->first_child; c; c = c->next_sibling)
		cui_build_draw_from_tree(ctx, c, buf, ox, oy);
}

void cui_draw_buf_init(cui_draw_command_buffer *buf, size_t capacity) {
	if (!buf || capacity == 0) return;
	buf->cmd = (cui_draw_cmd *)malloc(sizeof(cui_draw_cmd) * capacity);
	buf->capacity = buf->cmd ? capacity : 0;
	buf->count = 0;
}

void cui_draw_buf_fini(cui_draw_command_buffer *buf) {
	if (!buf) return;
	free(buf->cmd);
	buf->cmd = NULL;
	buf->capacity = 0;
	buf->count = 0;
}

void cui_draw_buf_clear(cui_draw_command_buffer *buf) {
	if (buf) buf->count = 0;
}

int cui_draw_buf_push_rect(cui_draw_command_buffer *buf, float x, float y, float w, float h, unsigned int color) {
	if (!buf || !buf->cmd || buf->count >= buf->capacity) return -1;
	buf->cmd[buf->count].type = CUI_CMD_RECT;
	buf->cmd[buf->count].u.fill_rect.x = x;
	buf->cmd[buf->count].u.fill_rect.y = y;
	buf->cmd[buf->count].u.fill_rect.w = w;
	buf->cmd[buf->count].u.fill_rect.h = h;
	buf->cmd[buf->count].u.fill_rect.color = color ? color : CUI_THEME_DEFAULT_BUTTON_BG;
	buf->count++;
	return 0;
}

int cui_draw_buf_push_line(cui_draw_command_buffer *buf, float x0, float y0, float x1, float y1, float thickness, unsigned int color) {
	if (!buf || !buf->cmd || buf->count >= buf->capacity) return -1;
	buf->cmd[buf->count].type = CUI_CMD_LINE;
	buf->cmd[buf->count].u.line.a.x = x0;
	buf->cmd[buf->count].u.line.a.y = y0;
	buf->cmd[buf->count].u.line.b.x = x1;
	buf->cmd[buf->count].u.line.b.y = y1;
	buf->cmd[buf->count].u.line.thickness = thickness;
	buf->cmd[buf->count].u.line.color = color;
	buf->count++;
	return 0;
}

int cui_draw_buf_push_text(cui_draw_command_buffer *buf, float x, float y, const char *text, unsigned int color) {
	if (!buf || !buf->cmd || buf->count >= buf->capacity || !text) return -1;
	buf->cmd[buf->count].type = CUI_CMD_TEXT;
	buf->cmd[buf->count].u.text.x = x;
	buf->cmd[buf->count].u.text.y = y;
	buf->cmd[buf->count].u.text.text = text;
	buf->cmd[buf->count].u.text.color = color;
	buf->count++;
	return 0;
}
