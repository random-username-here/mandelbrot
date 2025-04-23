#include "api.h"
#include <assert.h>
#include <stdbool.h>

#define BLOCK_SIZE 8

typedef struct { float d[BLOCK_SIZE]; } fblk_t;
typedef struct { int d[BLOCK_SIZE]; } iblk_t;

static inline fblk_t fblk_set(float val)
{
	fblk_t blk;
	for (int i = 0; i < BLOCK_SIZE; ++i)
		blk.d[i] = val;
	return blk;
}

static inline fblk_t fblk_add(fblk_t a, fblk_t b)
{
	fblk_t blk;
	for (int i = 0; i < BLOCK_SIZE; ++i)
		blk.d[i] = a.d[i] + b.d[i];
	return blk;
}

static inline fblk_t fblk_sub(fblk_t a, fblk_t b)
{
	fblk_t blk;
	for (int i = 0; i < BLOCK_SIZE; ++i)
		blk.d[i] = a.d[i] - b.d[i];
	return blk;
}

static inline fblk_t fblk_mul(fblk_t a, fblk_t b)
{
	fblk_t blk;
	for (int i = 0; i < BLOCK_SIZE; ++i)
		blk.d[i] = a.d[i] * b.d[i];
	return blk;
}

static inline iblk_t iblk_set(int val)
{
	iblk_t blk;
	for (int i = 0; i < BLOCK_SIZE; ++i)
		blk.d[i] = val;
	return blk;
}

void mandelbrot_arrays(struct Mb_GeneratorData *gen)
{
	float sheight = gen->swidth / gen->bwidth * gen->bheight;

	assert(gen->bwidth % 8 == 0);
	assert(__builtin_cpu_supports("avx2"));

	float DeltaRe0 = 1.0f / gen->bwidth * gen->swidth;
	fblk_t DeltaRe = { 0 };
	for (int i = 1; i < 8; ++i)
		DeltaRe.d[i] = DeltaRe.d[i-1] + DeltaRe0;

	float Radius2 = EXIT_RADIUS*EXIT_RADIUS;

	for (int iy = 0; iy < gen->bheight; ++iy) {
		float Im0_Val = (iy * 1.0f / gen->bheight - 0.5f) * sheight + gen->yc;
		fblk_t Im0 = fblk_set(Im0_Val);

		for (int ix = 0; ix < gen->bwidth; ix += 8) {

			float Re0_0 = (ix * 1.0f / gen->bwidth - 0.5f) * gen->swidth + gen->xc;

			fblk_t Re0 = fblk_add(DeltaRe, fblk_set(Re0_0));

			fblk_t ReN = Re0, ImN = Im0;

			iblk_t steps = iblk_set(0);

			for (int max_steps = 0; max_steps < gen->max_steps; max_steps++) {

				fblk_t ReN2 = fblk_mul(ReN, ReN);
				fblk_t ImN2 = fblk_mul(ImN, ImN);

				// ReSqr = ReN * ReN - ImN * ImN
				fblk_t Dist = fblk_add(ReN2, ImN2);

				// Mask those, which are inside the circle
				bool should_exit = true;
				for (int i = 0; i < BLOCK_SIZE; ++i)
					should_exit &= Dist.d[i] > Radius2;

				// If everyone is outside, exit
				if (should_exit)
					break;

				// Advance counter for ones inside
				for (int i = 0; i < BLOCK_SIZE; ++i)
					steps.d[i] += Dist.d[i] <= Radius2;

				// ImSqr = 2 * ReN * ImN
				fblk_t ImSqr;
				for (int i = 0; i < BLOCK_SIZE; ++i)
					ImSqr.d[i] = ReN.d[i] * ImN.d[i] * 2.0f;

				// ReN = ReSqr + Re0;
				// ImN = ImSqr + Im0;
				ReN = fblk_add(fblk_sub(ReN2, ImN2), Re0);
				ImN = fblk_add(ImSqr, Im0);

			}

			int *base = &gen->exit_steps[ix + iy*gen->bwidth];
			for (int i = 0; i < BLOCK_SIZE; ++i)
				base[i] = steps.d[i];
		}
	}
}

