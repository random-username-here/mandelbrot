#include "gen/api.h"

void mandelbrot_simple(struct Mb_GeneratorData *gen)
{
	float sheight = gen->swidth / gen->bwidth * gen->bheight;

	for (int iy = 0; iy < gen->bheight; ++iy) {
		for (int ix = 0; ix < gen->bwidth; ++ix) {

			float Re0 = (ix * 1.0f / gen->bwidth - 0.5) * gen->swidth + gen->xc;
			float Im0 = (iy * 1.0f / gen->bheight - 0.5) * sheight + gen->yc;

			float ReN = Re0, ImN = Im0;

			int steps = 0;
			for (; ReN*ReN + ImN*ImN < EXIT_RADIUS*EXIT_RADIUS && steps < gen->max_steps; ++steps) {

				// (a + ib) (a + ib) = a^2 + iba + iba - b^2 = (a^2 - b^2) + i 2ab
				float ReSqr = ReN*ReN - ImN*ImN;
				float ImSqr = 2*ReN*ImN;

				ReN = ReSqr + Re0;
				ImN = ImSqr + Im0;
			}

			gen->exit_steps[ix + iy*gen->bwidth] = steps;
		}
	}
}
