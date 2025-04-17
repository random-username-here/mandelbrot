#define FONT_IMPL

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "common.h"
#include "viewer.h"
#include "font.h"

void ui_fillrect(struct State *st, int x, int y, int w, int h, ARGB color)
{
	assert(st);

	for (int iy = y; iy < y+h && iy < WIN_HEIGHT; ++iy)
		for (int ix = x; ix < x+w && ix < WIN_WIDTH; ++ix)
			st->fb[iy * WIN_WIDTH + ix] = color;
}

void ui_textflow_init(struct UI_TextFlow *flow, struct State *state, int x, int y)
{
	assert(flow);
	assert(state);

	flow->state = state;
	flow->x = flow->base_x = x;
	flow->y = y;
	
}
void ui_textflow_puts(struct UI_TextFlow *flow, ARGB color, const char *text)
{
	for (;*text != '\0'; ++text) {
		if (flow->x + FONT_SIZE_X >= WIN_WIDTH || flow->y + FONT_SIZE_Y >= WIN_HEIGHT)
			continue;

		if (*text == '\n') {
			flow->x = flow->base_x;
			flow->y += FONT_LINE_GAP + FONT_SIZE_Y;
			continue;
		}

		if (*text < FONT_FIRST_CHAR)
			continue;

		for (int iy = 0; iy < FONT_SIZE_Y; ++iy)
			for (int ix = 0; ix < FONT_SIZE_X; ++ix)
				if (font_bitmap[*text - FONT_FIRST_CHAR][FONT_SIZE_Y-1-iy] & (1 << (FONT_SIZE_X-1-ix)))
					flow->state->fb[(flow->y + iy) * WIN_WIDTH + (flow->x + ix)] = color;
		flow->x += FONT_SIZE_X+1;
	}
}

__attribute__((format(printf, 3, 4)))
void ui_textflow_printf(struct UI_TextFlow *flow, ARGB color, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char msg[256] = {0};
	vsnprintf(msg, sizeof(msg)-1, fmt, args);

	ui_textflow_puts(flow, color, msg);

	va_end(args);
}
