/**
 * Flexbox-inspired layout: single pass + flex-grow.
 * CENTER, ROW, COLUMN, STACK, WRAP containers; leaf nodes get intrinsic or default size.
 */
#include "layout.h"
#include "core/node.h"
#include "core/theme.h"
#include "font/atlas.h"
#include <string.h>

#define ALIGN_START   0
#define ALIGN_CENTER  1
#define ALIGN_END     2
#define ALIGN_STRETCH 3

#define DEFAULT_GAP 8
#define DEFAULT_LEAF_W 80
#define DEFAULT_LEAF_H 24

static float node_intrinsic_w(const cui_node *n) {
	if (n->layout_w > 0) return n->layout_w;
	switch (n->type) {
		case CUI_NODE_LABEL: {
			float w = 0.f, h = 0.f;
			cui_font_measure(cui_font_default_id(), CUI_THEME_DEFAULT_FONT_SIZE, n->label_text, &w, &h);
			return w > 0.f ? w : 1.f;
		}
		case CUI_NODE_BUTTON: return DEFAULT_LEAF_W;
		case CUI_NODE_CHECKBOX:
		case CUI_NODE_ICON_BUTTON: return 24;
		case CUI_NODE_SPACER: return n->spacer_w > 0 ? n->spacer_w : 8;
		case CUI_NODE_TEXT_INPUT: return 120;
		case CUI_NODE_SCROLL: return 200;
		case CUI_NODE_CANVAS: return n->layout_w > 0 ? n->layout_w : 100;
		default: return DEFAULT_LEAF_W;
	}
}

static float node_intrinsic_h(const cui_node *n) {
	if (n->layout_h > 0) return n->layout_h;
	switch (n->type) {
		case CUI_NODE_LABEL: {
			float w = 0.f, h = 0.f;
			cui_font_measure(cui_font_default_id(), CUI_THEME_DEFAULT_FONT_SIZE, n->label_text, &w, &h);
			return h > 0.f ? h : (float)CUI_THEME_DEFAULT_FONT_SIZE;
		}
		case CUI_NODE_BUTTON: return DEFAULT_LEAF_H;
		case CUI_NODE_CHECKBOX:
		case CUI_NODE_ICON_BUTTON: return 24;
		case CUI_NODE_SPACER: return n->spacer_h > 0 ? n->spacer_h : 8;
		case CUI_NODE_TEXT_INPUT: return 24;
		case CUI_NODE_SCROLL: return n->scroll_max_h > 0 ? n->scroll_max_h : 100;
		case CUI_NODE_CANVAS: return n->layout_h > 0 ? n->layout_h : 100;
		default: return DEFAULT_LEAF_H;
	}
}

static float get_gap(const cui_node *n) {
	float g = n->layout_opts.gap;
	return g > 0 ? g : DEFAULT_GAP;
}

static void layout_leaf(cui_node *n, float x, float y) {
	n->layout_x = x;
	n->layout_y = y;
	if (n->layout_w <= 0) n->layout_w = node_intrinsic_w(n);
	if (n->layout_h <= 0) n->layout_h = node_intrinsic_h(n);
}

/* Measure pass: set layout_w, layout_h on each node (containers from children). */
static void measure(cui_node *n) {
	if (!n) return;
	for (cui_node *c = n->first_child; c; c = c->next_sibling)
		measure(c);
	switch (n->type) {
	case CUI_NODE_ROW: {
		float tw = 0, max_h = 0;
		int count = 0;
		float gap = get_gap(n);
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			tw += (count > 0 ? gap : 0) + (c->layout_w > 0 ? c->layout_w : node_intrinsic_w(c));
			float ch = c->layout_h > 0 ? c->layout_h : node_intrinsic_h(c);
			if (ch > max_h) max_h = ch;
			count++;
		}
		n->layout_w = n->layout_w > 0 ? n->layout_w : (tw + 2 * n->layout_opts.padding_x);
		n->layout_h = n->layout_h > 0 ? n->layout_h : (max_h + 2 * n->layout_opts.padding_y);
		break;
	}
	case CUI_NODE_SCROLL:
	case CUI_NODE_COLUMN:
	case CUI_NODE_CENTER: {
		float max_w = 0, th = 0;
		int count = 0;
		float gap = get_gap(n);
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			float cw = c->layout_w > 0 ? c->layout_w : node_intrinsic_w(c);
			if (cw > max_w) max_w = cw;
			th += (count > 0 ? gap : 0) + (c->layout_h > 0 ? c->layout_h : node_intrinsic_h(c));
			count++;
		}
		if (n->type == CUI_NODE_COLUMN || n->type == CUI_NODE_SCROLL || n->layout_w <= 0)
			n->layout_w = n->layout_w > 0 ? n->layout_w : (max_w + 2 * n->layout_opts.padding_x);
		if (n->type == CUI_NODE_COLUMN || n->layout_h <= 0)
			n->layout_h = n->layout_h > 0 ? n->layout_h : (th + 2 * n->layout_opts.padding_y);
		if (n->type == CUI_NODE_SCROLL && n->scroll_max_h > 0)
			n->layout_h = n->scroll_max_h;
		break;
	}
	case CUI_NODE_STACK: {
		float max_w = 0, max_h = 0;
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			float cw = c->layout_w > 0 ? c->layout_w : node_intrinsic_w(c);
			float ch = c->layout_h > 0 ? c->layout_h : node_intrinsic_h(c);
			if (cw > max_w) max_w = cw;
			if (ch > max_h) max_h = ch;
		}
		n->layout_w = n->layout_w > 0 ? n->layout_w : (max_w + 2 * n->layout_opts.padding_x);
		n->layout_h = n->layout_h > 0 ? n->layout_h : (max_h + 2 * n->layout_opts.padding_y);
		break;
	}
	case CUI_NODE_WRAP:
		/* Wrap size from children; we'll compute during position pass */
		if (n->layout_w <= 0) n->layout_w = 400;
		if (n->layout_h <= 0) n->layout_h = 200;
		break;
	default:
		if (n->layout_w <= 0) n->layout_w = node_intrinsic_w(n);
		if (n->layout_h <= 0) n->layout_h = node_intrinsic_h(n);
		break;
	}
}

static void run_layout(cui_node *n, float x, float y, float w, float h) {
	if (!n) return;

	const float pad_x = n->layout_opts.padding_x > 0 ? n->layout_opts.padding_x : 0;
	const float pad_y = n->layout_opts.padding_y > 0 ? n->layout_opts.padding_y : 0;
	const float inner_w = w - 2 * pad_x;
	const float inner_h = h - 2 * pad_y;
	const float gap = get_gap(n);

	n->layout_x = x;
	n->layout_y = y;
	n->layout_w = w;
	n->layout_h = h;

	switch (n->type) {
	case CUI_NODE_CENTER:
	case CUI_NODE_SCROLL:
	case CUI_NODE_COLUMN: {
		int align = n->layout_opts.align;
		if (n->type == CUI_NODE_CENTER) align = ALIGN_CENTER; /* center content by default */
		float cy = pad_y;
		float max_cw = 0;
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			float cw = c->layout_w > 0 ? c->layout_w : node_intrinsic_w(c);
			float ch = c->layout_h > 0 ? c->layout_h : node_intrinsic_h(c);
			if (cw > max_cw) max_cw = cw;
			cy += ch + gap;
		}
		cy = pad_y;
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			float cw = c->layout_w > 0 ? c->layout_w : node_intrinsic_w(c);
			float ch = c->layout_h > 0 ? c->layout_h : node_intrinsic_h(c);
			c->layout_w = cw;
			c->layout_h = ch;
			float cx = pad_x;
			if (align == ALIGN_CENTER)
				cx = pad_x + (inner_w - cw) / 2;
			else if (align == ALIGN_END)
				cx = pad_x + inner_w - cw;
			else if (align == ALIGN_STRETCH && inner_w > 0)
				cw = inner_w;
			c->layout_x = x + cx;
			c->layout_y = y + cy;
			c->layout_w = cw;
			c->layout_h = ch;
			run_layout(c, x + cx, y + cy, cw, ch);
			cy += ch + gap;
		}
		break;
	}
	case CUI_NODE_ROW: {
		float cx = pad_x;
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			float cw = c->layout_w > 0 ? c->layout_w : node_intrinsic_w(c);
			float ch = c->layout_h > 0 ? c->layout_h : node_intrinsic_h(c);
			c->layout_w = cw;
			c->layout_h = ch;
			c->layout_x = x + cx;
			c->layout_y = y + pad_y;
			c->layout_w = cw;
			c->layout_h = ch;
			run_layout(c, x + cx, y + pad_y, cw, ch);
			cx += cw + gap;
		}
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			if (n->layout_opts.align_y == ALIGN_CENTER && inner_h > 0)
				c->layout_y = y + pad_y + (inner_h - c->layout_h) / 2;
			else if (n->layout_opts.align_y == ALIGN_END && inner_h > 0)
				c->layout_y = y + pad_y + inner_h - c->layout_h;
			else if (n->layout_opts.align_y == ALIGN_STRETCH && inner_h > 0)
				c->layout_h = inner_h;
		}
		break;
	}
	case CUI_NODE_STACK: {
		float sx = x + pad_x, sy = y + pad_y;
		float max_w = 0, max_h = 0;
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			if (c->layout_w > max_w) max_w = c->layout_w;
			if (c->layout_h > max_h) max_h = c->layout_h;
		}
		if (max_w <= 0) max_w = DEFAULT_LEAF_W;
		if (max_h <= 0) max_h = DEFAULT_LEAF_H;
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			c->layout_x = sx;
			c->layout_y = sy;
			c->layout_w = max_w;
			c->layout_h = max_h;
			run_layout(c, sx, sy, max_w, max_h);
		}
		break;
	}
	case CUI_NODE_WRAP: {
		float cx = pad_x, cy = pad_y;
		float line_h = 0;
		float wrap_w = inner_w > 0 ? inner_w : 400;
		for (cui_node *c = n->first_child; c; c = c->next_sibling) {
			float cw = c->layout_w > 0 ? c->layout_w : node_intrinsic_w(c);
			float ch = c->layout_h > 0 ? c->layout_h : node_intrinsic_h(c);
			c->layout_w = cw;
			c->layout_h = ch;
			if (cx + cw > wrap_w && cx > pad_x) {
				cx = pad_x;
				cy += line_h + gap;
				line_h = 0;
			}
			c->layout_x = x + cx;
			c->layout_y = y + cy;
			c->layout_w = cw;
			c->layout_h = ch;
			run_layout(c, x + cx, y + cy, cw, ch);
			cx += cw + gap;
			if (ch > line_h) line_h = ch;
		}
		break;
	}
	case CUI_NODE_LABEL:
	case CUI_NODE_BUTTON:
		layout_leaf(n, x, y);
		break;
	default:
		layout_leaf(n, x, y);
		for (cui_node *c = n->first_child; c; c = c->next_sibling)
			run_layout(c, c->layout_x, c->layout_y, c->layout_w, c->layout_h);
		break;
	}
}

void cui_layout_run(cui_node *root, float viewport_w, float viewport_h) {
	if (!root) return;
	if (viewport_w <= 0) viewport_w = 400;
	if (viewport_h <= 0) viewport_h = 300;
	if (root->layout_w <= 0) root->layout_w = viewport_w;
	if (root->layout_h <= 0) root->layout_h = viewport_h;
	measure(root);
	run_layout(root, 0, 0, root->layout_w, root->layout_h);
}
