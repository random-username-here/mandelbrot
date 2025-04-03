#include "./api.h"
#include <x86intrin.h>
#include <assert.h>

void mandelbrot_avx(struct Mb_GeneratorData *gen)
{
	float sheight = gen->swidth / gen->bwidth * gen->bheight;

	assert(gen->bwidth % 4 == 0);
	assert(__builtin_cpu_supports("avx"));

	float DeltaRe0 = 1.0f / gen->bwidth * gen->swidth;
	float Re0Arr[4] = { 0 };
	for (int i = 1; i < 4; ++i)
		Re0Arr[i] = Re0Arr[i-1] + DeltaRe0;

	__m128 DeltaRe = _mm_load_ps(Re0Arr);
	__m128 Radius2 = _mm_set1_ps(EXIT_RADIUS*EXIT_RADIUS);
	__m128i m128i_One = _mm_set1_epi32(1);
	__m128 m128_Two = _mm_set1_ps(2);

	for (int iy = 0; iy < gen->bheight; ++iy) {
		float Im0_Val = (iy * 1.0f / gen->bheight - 0.5) * sheight + gen->yc;
		__m128 Im0 = _mm_set1_ps(Im0_Val);

		for (int ix = 0; ix < gen->bwidth; ix += 4) {

			float Re0_0 = (ix * 1.0f / gen->bwidth - 0.5) * gen->swidth + gen->xc;

			__m128 Re0 = _mm_add_ps(DeltaRe, _mm_set1_ps(Re0_0));

			__m128 ReN = Re0, ImN = Im0;

			__m128i steps = _mm_set1_epi32(0);

			for (int max_steps = 0; max_steps < gen->max_steps; max_steps++) {

				__m128 ReN2 = _mm_mul_ps(ReN, ReN);
				__m128 ImN2 = _mm_mul_ps(ImN, ImN);

				// ReSqr = ReN * ReN - ImN * ImN
				__m128 Dist = _mm_add_ps(ReN2, ImN2);

				// Mask those, which are inside the circle
				__m128 mask = _mm_cmp_ps(Dist, Radius2, _CMP_LT_OS);

				// If everyone is outside, exit
				if (!_mm_movemask_ps(mask))
					break;

				// Advance counter for ones inside
				__m128i delta = _mm_and_si128(m128i_One, _mm_castps_si128(mask));
				steps = _mm_add_epi32(steps, delta);

				// ImSqr = 2 * ReN * ImN
				__m128 ImSqr = _mm_mul_ps(
					m128_Two,
					_mm_mul_ps(ReN, ImN)
				);

				// ReN = ReSqr + Re0;
				// ImN = ImSqr + Im0;
				ReN = _mm_add_ps(_mm_sub_ps(ReN2, ImN2), Re0);
				ImN = _mm_add_ps(ImSqr, Im0);

			}

			_mm_store_si128((__m128i*) &gen->exit_steps[ix + iy*gen->bwidth], steps);
		}
	}
}
