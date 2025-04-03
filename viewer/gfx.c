#define FONT_IMPL
#include "./viewer.h"
#include "./font.h"

void ui_fillrect(struct State *st, int x, int y, int w, int h, ARGB color)
{
	for (int iy = y; iy < y+h && iy < st->h; ++iy)
		for (int ix = x; ix < x+w && ix < st->w; ++ix)
			st->fb[iy*st->w + ix] = color;
}

void ui_puts(struct State *st, int x, int y, ARGB color, const char *text)
{
	while (*text != '\0') {
		if (x + FONT_SIZE_X >= st->w || y + FONT_SIZE_Y >= st->h)
			continue;
		if (*text < FONT_FIRST_CHAR)
			continue;

		for (int iy = 0; iy < FONT_SIZE_Y; ++iy)
			for (int ix = 0; ix < FONT_SIZE_X; ++ix)
				if (font_bitmap[*text - FONT_FIRST_CHAR][FONT_SIZE_Y-1-iy] & (1 << (FONT_SIZE_X-1-ix)))
					st->fb[(y + iy) * st->w + (x + ix)] = color;
		x += FONT_SIZE_X+1;
		++text;
	}
}
