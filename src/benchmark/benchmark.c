#define _GNU_SOURCE

#include "common.h"
#include "gen/api.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <time.h>

#define PLOT_WIDTH 4 // 4 omega
#define SPLIT 16

#define PLOT_ROWS 7
#define PLOT_COLS (PLOT_WIDTH * SPLIT * 2 + 1)

static void init_gdata(struct Mb_GeneratorData *gdata)
{
	gdata->xc = gdata->yc = 0;
	gdata->swidth = 2;
	gdata->bwidth = WIN_WIDTH;
	gdata->bheight = WIN_HEIGHT;
	gdata->exit_steps = aligned_alloc(
			32, gdata->bwidth * gdata->bheight * sizeof(*gdata->exit_steps)
	);
	gdata->max_steps = 255;
}

void print_usage(const char *name)
{
	printf(
			"Usage: %s -g GENERATOR_NAME [-m MEASURE_WIN_W]"
			" [-v MAX_VARIATION] [-h]\n", name
	);
	printf(
			"  -h                 Prints this help message\n"
			"  -g GENERATOR_NAME  Choses which algorithm to use for computation:\n"
			"                     Currently availiable: "
	);
	for (int i = 0; i < ARRAY_SIZE(generators); ++i)
		printf("%s ", generators[i].name);
	printf(
			"\n"
			"  -m MEASURE_WIN_W   Number of measurements to average in the result\n"
			"  -v MAX_VARIATION   Maximum relative difference in time between min\n"
			"                     and max time to say what measuremenets are stable\n"
	);
}

int main(int argc, char **argv)
{

	int measure_window = 32;
	float acceptable_var = 1.05;
	const char *gen_name = NULL;

	int opt;
	while ((opt = getopt(argc, argv, "g:m:v:h")) != -1) {
		switch (opt) {
		case 'h':
			print_usage(argv[0]);
			return 0;
		case 'g':
			gen_name = optarg;
			break;
		case 'm':
			measure_window = atoi(optarg);
			break;
		case 'v':
			acceptable_var = atof(optarg);
			break;
		default:
			printf("Unknown option `%c`\n", opt);
			return -1;
		}
	}

	if (!gen_name) {
		printf("Please chose a generator name, you can see list of them in `-h`\n");
		return -1;
	}

	void (*mandelbrot)(struct Mb_GeneratorData *gdata) = NULL;
	for (int i = 0; i < ARRAY_SIZE(generators); ++i)
		if (strcmp(generators[i].name, gen_name) == 0)
			mandelbrot = generators[i].mandelbrot;

	if (!mandelbrot) {
		printf("There is no generator named `%s`\n", gen_name);
		return -1;
	}

	if (measure_window <= 1) {
		printf("`-m` expects a positive integer number\n");
		return -1;
	}

	if (isnan(acceptable_var) || isinf(acceptable_var) || acceptable_var < 1) {
		printf("Acceptable variation must be float number which is > 1\n");
		return -1;
	}

	struct Mb_GeneratorData gdata;
	init_gdata(&gdata);

	float *times = calloc(measure_window, sizeof(*times));

	printf("## Starting benchmark\n\n");
	printf("Acceptable variation: %f\n", acceptable_var);
	printf("Measure window width: %d\n", measure_window);
	printf("Algorithm: %s\n", gen_name);

	printf("## Running benchmark\n\n");

	bool ok = false;
	int runs = 0;

	for (; runs < measure_window || !ok; ++runs) {
		clock_t begin = clock();
		mandelbrot(&gdata);
		float this_time = (clock() - begin) * 1.0f / CLOCKS_PER_SEC * 1000;
		times[runs % measure_window] = this_time;

		float minv = INFINITY, maxv = -INFINITY;
		for (int j = 0; j < measure_window; ++j) {
			if (times[j] < minv) minv = times[j];
			if (times[j] > maxv) maxv = times[j];
		}

		printf(
				"Run %-4d -- time is %-8.4f. In last %-4d runs -- min %-8.2f, max %-8.2f -- var %-5.4f\n",
				runs, this_time, measure_window, minv, maxv, maxv / minv
		);
		ok = minv * acceptable_var >= maxv;
		if (ok)
			printf("The thing is stable enough\n");
	}
	printf("\n\n");

	float avg = 0;
	for (int i = 0; i < measure_window; ++i)
		avg += times[i];
	avg /= measure_window;

	float dev = 0;
	for (int i = 0; i < measure_window; ++i)
		dev += powf(times[i] - avg, 2);
	dev /= measure_window;
	dev = sqrtf(dev);

	printf("## Benchmark results:\n\n");
	printf(
			"Done %d warmup runs and %d measurment runs\n",
			runs - measure_window, measure_window
	);
	printf("Time avg %f ms, std dev %f ms\n", avg, dev);
	printf("With 𝜎 (68%% probability) time is %f ± %f ms\n", avg, dev);
	printf("With 3𝜎 (99.73%% probability) time is %f ± %f ms\n", avg, dev*3);

	//------------------------------------------------------
	// Check for runs which are out of 3 std. dev.
	// from average
	
	float max_err = 0, max_outlier = 0;
	int num_outliers = 0;
	for (int i = 0; i < measure_window; ++i) {
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
	printf(
			"Non-outlier runs are withn %0.3f𝜎, outliers (>3𝜎) are withn %0.3f𝜎\n",
			max_err, max_outlier
	);

	if (num_outliers == 0)
		{}
	else if (num_outliers <= measure_window / 20)
		printf("Warn: Some values are out 3𝜎, but there are a few of them\n");
	else
		printf("Err: > 5%% of values are out of 3𝜎, measurement failed\n");

	//------------------------------------------------------
	// The plot

	printf("\nDistribution plot:\n");
	printf("One tick = one std. dev = %f ms\n\n", dev);

	int cols[PLOT_COLS] = {0};

	const float col_w = dev / SPLIT;

	for (int i = 0; i < measure_window; ++i) {
		float base = times[i] - (avg - PLOT_COLS * col_w / 2);
		int at = floorf(base / col_w);
		if (at < 0 || at >= PLOT_COLS) // out of the plot
			continue;
		cols[at]++;
	}

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


	//------------------------------------------------------
	// Cleanup

	free(gdata.exit_steps);
	free(times);
	return 0;
}
