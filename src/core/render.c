/**
 * Full draw command submission: take context draw buffer, apply scale (Hi-DPI),
 * submit to RDI and present. Same buffer format consumed by software and GPU drivers.
 */
#include "core/context.h"
#include "core/draw_cmd.h"
#include "clearui_rdi.h"
#include <string.h>

static void scale_buf(const cui_draw_command_buffer *src, cui_draw_command_buffer *dst, float scale) {
	if (!src || !dst || scale <= 0) return;
	dst->count = 0;
	for (size_t i = 0; i < src->count && dst->count < dst->capacity; i++) {
		const cui_draw_cmd *c = &src->cmd[i];
		cui_draw_cmd *out = &dst->cmd[dst->count];
		out->type = c->type;
		switch (c->type) {
		case CUI_CMD_RECT:
			out->u.fill_rect.x = c->u.fill_rect.x * scale;
			out->u.fill_rect.y = c->u.fill_rect.y * scale;
			out->u.fill_rect.w = c->u.fill_rect.w * scale;
			out->u.fill_rect.h = c->u.fill_rect.h * scale;
			out->u.fill_rect.color = c->u.fill_rect.color;
			break;
		case CUI_CMD_LINE:
			out->u.line.a.x = c->u.line.a.x * scale;
			out->u.line.a.y = c->u.line.a.y * scale;
			out->u.line.b.x = c->u.line.b.x * scale;
			out->u.line.b.y = c->u.line.b.y * scale;
			out->u.line.thickness = c->u.line.thickness * scale;
			out->u.line.color = c->u.line.color;
			break;
		case CUI_CMD_TEXT:
			out->u.text.x = c->u.text.x * scale;
			out->u.text.y = c->u.text.y * scale;
			out->u.text.text = c->u.text.text;
			out->u.text.color = c->u.text.color;
			break;
		case CUI_CMD_ROUNDED_RECT:
			out->u.rounded_rect.x = c->u.rounded_rect.x * scale;
			out->u.rounded_rect.y = c->u.rounded_rect.y * scale;
			out->u.rounded_rect.w = c->u.rounded_rect.w * scale;
			out->u.rounded_rect.h = c->u.rounded_rect.h * scale;
			out->u.rounded_rect.r = c->u.rounded_rect.r * scale;
			out->u.rounded_rect.color = c->u.rounded_rect.color;
			break;
		case CUI_CMD_SCISSOR:
			out->u.scissor.x = c->u.scissor.x * scale;
			out->u.scissor.y = c->u.scissor.y * scale;
			out->u.scissor.w = c->u.scissor.w * scale;
			out->u.scissor.h = c->u.scissor.h * scale;
			break;
		default:
			memcpy(&out->u, &c->u, sizeof(out->u));
			break;
		}
		dst->count++;
	}
}

void cui_render_submit(cui_ctx *ctx) {
	const cui_rdi *rdi = (const cui_rdi *)cui_ctx_rdi(ctx);
	void *rdi_ctx = cui_ctx_rdi_ctx(ctx);
	if (!ctx || !rdi) return;
	const cui_config *config = cui_ctx_config(ctx);
	float scale = config && config->scale_factor > 0 ? config->scale_factor : 1.f;
	const cui_draw_command_buffer *buf = cui_ctx_draw_buf(ctx);
	if (scale == 1.f) {
		if (rdi->submit && rdi_ctx)
			rdi->submit(rdi_ctx, buf);
	} else {
		cui_draw_command_buffer *scaled = cui_ctx_scaled_buf(ctx);
		if (scaled) {
			scale_buf(buf, scaled, scale);
			if (rdi->submit && rdi_ctx)
				rdi->submit(rdi_ctx, scaled);
		}
	}
	if (rdi->present && rdi_ctx)
		rdi->present(rdi_ctx);
}
