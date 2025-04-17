///
/// API for mandelbrot set calculator
/// Coloring is not implemented here
///
#ifndef I_GEN_API
#define I_GEN_API

#include <stdint.h>

#define EXIT_RADIUS 10

struct Mb_GeneratorData {

	int *exit_steps;
	int bwidth, bheight;
	int max_steps;

	float xc, yc;
	float swidth;
};

struct Mb_Generator {
	void (*mandelbrot)(struct Mb_GeneratorData *gen);
	const char *name;
};

void mandelbrot_simple(struct Mb_GeneratorData *gen);
void mandelbrot_avx2(struct Mb_GeneratorData *gen);
void mandelbrot_avx(struct Mb_GeneratorData *gen);

static const struct Mb_Generator generators[] = {
	{ mandelbrot_simple, "simple" },
	{ mandelbrot_avx, "avx" },
	{ mandelbrot_avx2, "avx2" }
};

#define DEFAULT_GENERATOR 2

#endif
