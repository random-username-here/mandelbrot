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
	int *exit_steps_alt;
	int bwidth, bheight;
	int max_steps;

	float xc, yc;
	float swidth;

	float nxc, nyc, nswidth;
};

void mandelbrot_simple(struct Mb_GeneratorData *gen);
void mandelbrot_avx2(struct Mb_GeneratorData *gen);
void mandelbrot_avx(struct Mb_GeneratorData *gen);

#endif
