#define _GNU_SOURCE
#include <math.h>
#include "viewer.h"
#include <stdbool.h>

#define PLOT_WIDTH 4 // 4 omega
#define SPLIT 16

#define PLOT_ROWS 7
#define PLOT_COLS (PLOT_WIDTH * SPLIT * 2 + 1)

static void init_gdata(struct Mb_GeneratorData *gdata)
{
	gdata->xc = gdata->yc = 0;
	gdata->swidth = 2;
	gdata->bwidth = DFLT_WIDTH;
	gdata->bheight = DFLT_HEIGHT;
	gdata->exit_steps = aligned_alloc(32, gdata->bwidth * gdata->bheight * sizeof(*gdata->exit_steps));
	gdata->max_steps = 255;
}

void do_benchmark(struct Mb_BenchmarkState *state)
{
	struct Mb_GeneratorData gdata;
	init_gdata(&gdata);

	float *times = calloc(state->num_runs, sizeof(*times));

	printf("Acceptable variation: %f\n", state->acceptable_var);
	printf("## Running benchmark\n\n");

	bool ok = false;

	int runs = 0;

	for (; runs < state->num_runs || !ok; ++runs) {
		clock_t begin = clock();
		state->mandelbrot(&gdata);
		float run = (clock() - begin) * 1.0f / CLOCKS_PER_SEC * 1000;
		times[runs % state->num_runs] = run;

		printf("Run #%d -- time is %f%*s\n", runs, run, 80, "");

		float minv = INFINITY, maxv = -INFINITY;
		for (int j = 0; j < state->num_runs; ++j) {
			if (times[j] < minv) minv = times[j];
			if (times[j] > maxv) maxv = times[j];
		}

		printf("In last %d runs -- min %f, max %f -- var %f\n", state->num_runs, minv, maxv, maxv / minv);
		ok = minv * state->acceptable_var >= maxv;
		if (ok)
			printf("The thing is stable enough\n");
	}
	printf("\n\n");

	int num_runs = state->num_runs;

	float avg = 0;
	for (int i = 0; i < num_runs; ++i)
		avg += times[i];
	avg /= num_runs;

	float dev = 0;
	for (int i = 0; i < num_runs; ++i)
		dev += powf(times[i] - avg, 2);
	dev /= num_runs;
	dev = sqrtf(dev);

	printf("## Benchmark results:\n\n");
	printf("Done %d warmup runs and %d measurment runs\n", runs - num_runs, num_runs);
	printf("Time avg %f ms, std dev %f ms\n", avg, dev);
	printf("With 𝜎 (68%% probability) time is %f ± %f ms\n", avg, dev);
	printf("With 3𝜎 (99.73%% probability) time is %f ± %f ms\n", avg, dev*3);

	float max_err = 0, max_outlier = 0;
	int num_outliers = 0;
	for (int i = 0; i < num_runs; ++i) {
		float rel_err = fabs(times[i] - avg) / dev;
		if (rel_err > 3) {
			printf("Warn: Run %d has distance to center %f𝜎\n", i+1, rel_err);
			max_outlier = fmax(rel_err, max_outlier);
			++num_outliers;
		} else {
			max_err = fmax(rel_err, max_err);
		}
	}
	printf("Have %d outliers\n", num_outliers);
	printf("Non-outlier runs are withn %0.3f𝜎, outliers (>3𝜎) are withn %0.3f𝜎\n", max_err, max_outlier);

	if (num_outliers == 0)
		{}
	else if (num_outliers <= num_runs / 20)
		printf("Warn: Some values are out 3𝜎, but there are a few of them\n");
	else
		printf("Err: > 5%% of values are out of 3𝜎, measurement failed\n");

	printf("\nDistribution plot:\n");
	printf("One tick = one std. dev = %f ms\n\n", dev);

	int cols[PLOT_COLS] = {0};

	const float col_w = dev / SPLIT;

	for (int i = 0; i < num_runs; ++i) {
		float base = times[i] - (avg - PLOT_COLS * col_w / 2);
		int at = floorf(base / col_w);
		if (at < 0 || at >= PLOT_COLS) // out of the plot
			continue;
		cols[at]++;
	}

	/*printf("counts:\n");
	for (int i = 0; i < PLOT_COLS; ++i)
		printf("%d ", cols[i]);
	printf("\n");*/

	int maxv = 0;
	for (int i = 0; i < PLOT_COLS; ++i)
		if (maxv < cols[i])
			maxv = cols[i];

	for (int i = 0; i < PLOT_ROWS; ++i) {
		printf("  ");
		for (int j = 0; j < PLOT_COLS; ++j) {
			float level = cols[j] * 1.0f * PLOT_ROWS / maxv;
			if (level > PLOT_ROWS - i - 1)
				printf("#");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("-");
	for (int i = 0; i < PLOT_WIDTH * 2; ++i) {
		printf("+");
		for (int j = 0; j < SPLIT-1; ++j)
			printf("-");
	}
	printf("+-> time\n");
	printf("%*s| avg = %f ms", 1 + SPLIT * PLOT_WIDTH, "", avg);
	
}
