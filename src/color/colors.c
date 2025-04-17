#include "color/api.h"
#include <math.h>

ARGB color_grayscale(int steps, int max_steps)
{
	if (steps == max_steps)
		steps = 0;
	float st = steps * 1.0f / max_steps;
	float sn = sqrtf(st);
	uint8_t c = sn * 255;
	return RGB(c, c, c);
}


ARGB color_red_yellow(int steps, int max_steps)
{
	// formulas from
	// https://github.com/ralex2304/AvxMandelbrot
	// altho here components are mixed somehow??
	// anyways, this way it also looks interesting
	int norm = steps * 255 / max_steps;
	return RGB(
		255 - norm,
		((255 - norm) % 8) * 32,
		((255 - norm) % 2) * 255
	);
}

ARGB color_blue(int steps, int max_steps)
{
	float t = steps * 1.0f / max_steps;

	ARGB col = RGB(
		0,
		(uint8_t) (powf(fmax(0, t - 0.25) / (1 - 0.25), 0.5) * 255),
		(uint8_t) (powf(t, 0.5) * 255)
	);

	// smooth transition into black
	float fact = 1 - powf(fmax(0, t - 0.75) / (1 - 0.75), 4);
	col.r *= fact;
	col.g *= fact;
	col.b *= fact;

	return col;
}
