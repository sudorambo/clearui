#ifndef CLEARUI_DRAW_CMD_H
#define CLEARUI_DRAW_CMD_H

#include <stddef.h>

typedef enum cui_cmd_type {
	CUI_CMD_NOP,
	CUI_CMD_RECT,
	CUI_CMD_ROUNDED_RECT,
	CUI_CMD_LINE,
	CUI_CMD_TEXT,
	CUI_CMD_SCISSOR,
	CUI_CMD_COUNT
} cui_cmd_type;

typedef struct { float x, y, w, h; } cui_rect;
typedef struct { float x, y, w, h; unsigned int color; } cui_cmd_fill_rect;
typedef struct { float x, y; } cui_vec2;

typedef struct {
	float x, y, w, h;
	float r;
	unsigned int color;
} cui_cmd_rounded_rect;

typedef struct {
	cui_vec2 a, b;
	float thickness;
	unsigned int color;
} cui_cmd_line;

typedef struct {
	float x, y;
	const char *text;
	unsigned int color;
} cui_cmd_text;

typedef struct {
	cui_cmd_type type;
	union {
		cui_rect rect;
		cui_cmd_fill_rect fill_rect;
		cui_cmd_rounded_rect rounded_rect;
		cui_cmd_line line;
		cui_cmd_text text;
		cui_rect scissor;
	} u;
} cui_draw_cmd;

typedef struct cui_draw_command_buffer {
	cui_draw_cmd *cmd;
	size_t       capacity;
	size_t       count;
} cui_draw_command_buffer;

void cui_draw_buf_init(cui_draw_command_buffer *buf, size_t capacity);
void cui_draw_buf_fini(cui_draw_command_buffer *buf);
void cui_draw_buf_clear(cui_draw_command_buffer *buf);
int  cui_draw_buf_push_rect(cui_draw_command_buffer *buf, float x, float y, float w, float h, unsigned int color);
int  cui_draw_buf_push_line(cui_draw_command_buffer *buf, float x0, float y0, float x1, float y1, float thickness, unsigned int color);
int  cui_draw_buf_push_text(cui_draw_command_buffer *buf, float x, float y, const char *text, unsigned int color);
int  cui_draw_buf_push_rounded_rect(cui_draw_command_buffer *buf, float x, float y, float w, float h, float r, unsigned int color);
int  cui_draw_buf_push_scissor(cui_draw_command_buffer *buf, float x, float y, float w, float h);

struct cui_node;
struct cui_ctx;
void cui_build_draw_from_tree(struct cui_ctx *ctx, struct cui_node *root, cui_draw_command_buffer *buf, float offset_x, float offset_y);

#endif
