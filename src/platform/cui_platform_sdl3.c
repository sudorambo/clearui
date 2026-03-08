/**
 * SDL3 platform adapter: window, events (mouse/key/scroll), clipboard, cursor, scale.
 * Built only when WITH_SDL3=1. Uses SDL3 C API; app owns the loop.
 */
#include "../../include/clearui_platform_sdl3.h"
#include "../../include/clearui.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

typedef struct sdl3_ctx {
	SDL_Window   *window;
	SDL_Renderer *renderer;
	SDL_Texture  *texture;
	int width;
	int height;
	int tex_w;
	int tex_h;
} sdl3_ctx;

static int window_create(cui_platform_ctx **out_ctx, const char *title, int width, int height) {
	if (!out_ctx) return -1;
	*out_ctx = NULL;
	if (!SDL_Init(SDL_INIT_VIDEO))
		return -1;
	SDL_Window *win = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
	if (!win) {
		SDL_Quit();
		return -1;
	}
	sdl3_ctx *c = (sdl3_ctx *)malloc(sizeof(sdl3_ctx));
	if (!c) {
		SDL_DestroyWindow(win);
		SDL_Quit();
		return -1;
	}
	memset(c, 0, sizeof(*c));
	c->window = win;
	c->width = width;
	c->height = height;
	c->renderer = SDL_CreateRenderer(win, NULL);
	if (!c->renderer) {
		SDL_DestroyWindow(win);
		SDL_Quit();
		free(c);
		return -1;
	}
	*out_ctx = (cui_platform_ctx *)c;
	return 0;
}

static void window_destroy(cui_platform_ctx *ctx) {
	if (!ctx) return;
	sdl3_ctx *c = (sdl3_ctx *)ctx;
	if (c->texture) {
		SDL_DestroyTexture(c->texture);
		c->texture = NULL;
	}
	if (c->renderer) {
		SDL_DestroyRenderer(c->renderer);
		c->renderer = NULL;
	}
	if (c->window) {
		SDL_DestroyWindow(c->window);
		c->window = NULL;
	}
	SDL_Quit();
	free(c);
}

static void window_get_size(cui_platform_ctx *ctx, int *width, int *height) {
	if (!ctx) return;
	sdl3_ctx *c = (sdl3_ctx *)ctx;
	if (c->window) {
		SDL_GetWindowSize(c->window, &c->width, &c->height);
	}
	if (width) *width = c->width;
	if (height) *height = c->height;
}

static bool poll_events(cui_platform_ctx *ctx, void *app_ctx) {
	if (!ctx) return true;
	cui_ctx *ctx_ui = (cui_ctx *)app_ctx;

	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_EVENT_QUIT:
			return false;
		case SDL_EVENT_MOUSE_MOTION:
			if (ctx_ui)
				cui_inject_mouse_move(ctx_ui, (int)e.motion.x, (int)e.motion.y);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (ctx_ui && e.button.button == SDL_BUTTON_LEFT)
				cui_inject_click(ctx_ui, (int)e.button.x, (int)e.button.y);
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			if (ctx_ui)
				cui_inject_scroll(ctx_ui, (int)e.wheel.x, (int)e.wheel.y);
			break;
		case SDL_EVENT_KEY_DOWN:
			if (ctx_ui) {
				SDL_Keycode key = e.key.key;
				int k = 0;
				if (key == SDLK_TAB) k = (e.key.mod & SDL_KMOD_SHIFT) ? 0x0101 : 0x0100;
				else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) k = 0x0102;
				else if (key == SDLK_SPACE) k = 0x0103;
				else if (key == SDLK_BACKSPACE) k = 0x0104;
				else if (key == SDLK_DELETE) k = 0x0105;
				if (k) cui_inject_key(ctx_ui, k);
			}
			break;
		case SDL_EVENT_TEXT_INPUT:
			if (ctx_ui && e.text.text[0]) {
				unsigned int cp = (unsigned char)e.text.text[0];
				if (cp >= 32 && cp <= 126)
					cui_inject_char(ctx_ui, cp);
			}
			break;
		default:
			break;
		}
	}
	return true;
}

static int clipboard_get(cui_platform_ctx *ctx, char *buf, size_t size) {
	(void)ctx;
	if (!buf || size == 0) return -1;
	char *text = SDL_GetClipboardText();
	if (!text) return -1;
	size_t len = strlen(text);
	if (len >= size) len = size - 1;
	memcpy(buf, text, len);
	buf[len] = '\0';
	SDL_free(text);
	return 0;
}

static int clipboard_set(cui_platform_ctx *ctx, const char *text) {
	(void)ctx;
	if (!text) return -1;
	return SDL_SetClipboardText(text) ? -1 : 0;
}

static void cursor_set(cui_platform_ctx *ctx, int shape) {
	(void)ctx;
	/* SDL3 renamed: IBEAM→TEXT, ARROW→DEFAULT */
	SDL_SystemCursor id = (shape == CUI_CURSOR_TEXT) ? SDL_SYSTEM_CURSOR_TEXT : SDL_SYSTEM_CURSOR_DEFAULT;
	SDL_Cursor *cur = SDL_CreateSystemCursor(id);
	if (cur)
		SDL_SetCursor(cur);
}

static float scale_factor_get(cui_platform_ctx *ctx) {
	if (!ctx) return 1.f;
	sdl3_ctx *c = (sdl3_ctx *)ctx;
	if (!c->window) return 1.f;
	float scale = SDL_GetWindowPixelDensity(c->window);
	return scale >= 1.f ? scale : 1.f;
}

static void present_software(cui_platform_ctx *ctx, const void *rgba, int width, int height, int pitch_bytes) {
	if (!ctx || !rgba || width <= 0 || height <= 0) return;
	sdl3_ctx *c = (sdl3_ctx *)ctx;
	if (!c->renderer) return;
	if (!c->texture || c->tex_w != width || c->tex_h != height) {
		if (c->texture) SDL_DestroyTexture(c->texture);
		c->texture = SDL_CreateTexture(c->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
		c->tex_w = width;
		c->tex_h = height;
		if (!c->texture) return;
	}
	if (SDL_UpdateTexture(c->texture, NULL, rgba, pitch_bytes) != 0) return;
	SDL_RenderClear(c->renderer);
	SDL_RenderTexture(c->renderer, c->texture, NULL, NULL);
	SDL_RenderPresent(c->renderer);
}

static cui_platform sdl3_platform = {
	.window_create = window_create,
	.window_destroy = window_destroy,
	.window_get_size = window_get_size,
	.poll_events = poll_events,
	.clipboard_get = clipboard_get,
	.clipboard_set = clipboard_set,
	.cursor_set = cursor_set,
	.scale_factor_get = scale_factor_get,
	.surface_get = NULL,
	.surface_destroy = NULL,
	.present_software = present_software,
};

const cui_platform *cui_platform_sdl3_get(void) {
	return &sdl3_platform;
}
