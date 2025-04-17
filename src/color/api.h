#ifndef I_COLOR_API
#define I_COLOR_API

#include <stdint.h>

struct __attribute__((packed)) ARGB {
	uint8_t b, g, r, a;
};
typedef struct ARGB ARGB;

struct Mb_Colorizer {
	ARGB (*color)(int steps, int max_steps);
	const char *name;
};


#define RGB(_r, _g, _b) ((ARGB) { .r = (_r), .g = (_g), .b = (_b) })

ARGB color_grayscale(int steps, int max_steps);
ARGB color_red_yellow(int steps, int max_steps);
ARGB color_blue(int steps, int max_steps);

static struct Mb_Colorizer colorizers[] = {
	{ color_grayscale, "grayscale" },
	{ color_red_yellow, "red-yellow" },
	{ color_blue, "blue" },
};

#define DEFAULT_COLORIZER 0

#endif
