#include "./api.h"
#include <x86intrin.h>
#include <assert.h>

void mandelbrot_avx2(struct Mb_GeneratorData *gen)
{
	float sheight = gen->swidth / gen->bwidth * gen->bheight;

	assert(gen->bwidth % 8 == 0);
	assert(__builtin_cpu_supports("avx2"));

	float DeltaRe0 = 1.0f / gen->bwidth * gen->swidth;
	float Re0Arr[8] = { 0 };
	for (int i = 1; i < 8; ++i)
		Re0Arr[i] = Re0Arr[i-1] + DeltaRe0;

	__m256 DeltaRe = _mm256_load_ps(Re0Arr);
	__m256 Radius2 = _mm256_set1_ps(EXIT_RADIUS*EXIT_RADIUS);
	__m256i m256i_One = _mm256_set1_epi32(1);
	__m256 m256_Two = _mm256_set1_ps(2);

	for (int iy = 0; iy < gen->bheight; ++iy) {
		float Im0_Val = (iy * 1.0f / gen->bheight - 0.5) * sheight + gen->yc;
		__m256 Im0 = _mm256_set1_ps(Im0_Val);

		for (int ix = 0; ix < gen->bwidth; ix += 8) {

			float Re0_0 = (ix * 1.0f / gen->bwidth - 0.5) * gen->swidth + gen->xc;

			__m256 Re0 = _mm256_add_ps(DeltaRe, _mm256_set1_ps(Re0_0));

			__m256 ReN = Re0, ImN = Im0;

			//int steps = 0;
			__m256i steps = _mm256_set1_epi32(0);

			for (int max_steps = 0; max_steps < gen->max_steps; max_steps++) {

				__m256 ReN2 = _mm256_mul_ps(ReN, ReN);
				__m256 ImN2 = _mm256_mul_ps(ImN, ImN);

				// ReSqr = ReN * ReN - ImN * ImN
				__m256 Dist = _mm256_add_ps(ReN2, ImN2);

				// Mask those, which are inside the circle
				__m256 mask = _mm256_cmp_ps(Dist, Radius2, _CMP_LT_OS);

				// If everyone is outside, exit
				if (!_mm256_movemask_ps(mask))
					break;

				// Advance counter for ones inside
				__m256i delta = _mm256_and_si256(m256i_One, _mm256_castps_si256(mask));
				steps = _mm256_add_epi32(steps, delta);

				// ImSqr = 2 * ReN * ImN
				__m256 ImSqr = _mm256_mul_ps(
					m256_Two,
					_mm256_mul_ps(ReN, ImN)
				);

				// ReN = ReSqr + Re0;
				// ImN = ImSqr + Im0;
				ReN = _mm256_add_ps(_mm256_sub_ps(ReN2, ImN2), Re0);
				ImN = _mm256_add_ps(ImSqr, Im0);

			}

			_mm256_store_si256((__m256i*) &gen->exit_steps[ix + iy*gen->bwidth], steps);
		}
	}
}
