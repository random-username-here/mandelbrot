#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <bits/getopt_core.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <wchar.h>
#define VIEWER
#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#include "./viewer.h"
#include "./font.h"

#define DFLT_WIDTH 1024
#define DFLT_HEIGHT 768
#define ARROW_STEP_DIST 0.1

#define DIE(fmt, ...) \
	do {\
		fprintf(stderr, "Error: " fmt "\n" __VA_OPT__(,) __VA_ARGS__);\
		exit(EXIT_FAILURE);\
	} while (0)

static volatile bool shall_quit = false;
static pthread_mutex_t push_mutex;
static float render_time;
static void (*mandelbrot)(struct Mb_GeneratorData *gdata);

void render_thread(struct State *state)
{
	while (!shall_quit) {
		state->gdata.xc = state->gdata.nxc;
		state->gdata.yc = state->gdata.nyc;
		state->gdata.swidth = state->gdata.nswidth;
		clock_t begin = clock();
		mandelbrot(&state->gdata);
		float time = (clock() - begin) * 1.0f / CLOCKS_PER_SEC * 1000;

		pthread_mutex_lock(&push_mutex);
		SWAP(state->gdata.exit_steps, state->gdata.exit_steps_alt);
		render_time = time;
		pthread_mutex_unlock(&push_mutex);
	}
}

static inline ARGB shade(int step) {
	if (step == 255)
		step = 0;
	float st = step / 255.0f;
	float sn = sqrtf(st);
	uint8_t c = sn * 255;
	return RGB(c, c, c);
}

#define BAR_W 16

void do_benchmark(int num_runs, int num_startup)
{
	struct Mb_GeneratorData gdata;
	gdata.xc = gdata.yc = 0;
	gdata.swidth = 2;
	gdata.bwidth = DFLT_WIDTH;
	gdata.bheight = DFLT_HEIGHT;
	gdata.exit_steps = aligned_alloc(32, gdata.bwidth * gdata.bheight * sizeof(*gdata.exit_steps));
	gdata.max_steps = 255;

	float *times = calloc(num_runs + num_startup, sizeof(*times));

	printf("## Benchmark parameters:\n\n");
	printf(" - Warmup runs: %d\n", num_startup);
	printf(" - Main runs: %d\n", num_runs);
	printf("\n");
	printf("## Running benchmark\n\n");

	for (int i = 0; i < num_startup + num_runs; ++i) {
		clock_t begin = clock();
		mandelbrot(&gdata);
		float run = (clock() - begin) * 1.0f / CLOCKS_PER_SEC * 1000;
		times[i] = run;

		printf("\r%s run #%d -- time is %f%*s\n", 
				(i < num_startup ? "Startup" : "Main"), (i < num_startup ? i : i - num_startup) + 1, run, 80, "");

		printf("\r[");
		for (int j = 0; j < BAR_W; ++j)
			putc((j > (i+1) * 1.0 / (num_startup + num_runs) * BAR_W) ? '-' : '#', stdout);
		printf("] %2d / %2d -- measuring...", i+1, num_runs + num_startup);
		fflush(stdout);
	}
	printf("\n\n");

	float time = 0, sqtime = 0;
	for (int i = 0; i < num_runs; ++i) {
		float t = times[i+num_startup];
		time += t;
		sqtime += t * t;
	}

	float avg = time / num_runs;
	float dev = sqrtf(sqtime / num_runs - avg * avg);

	printf("## Benchmark results:\n\n");
	printf("Done %d warmup runs and %d measurment runs\n", num_startup, num_runs);
	printf("Time avg %f ms, std dev %f ms\n", avg, dev);
	printf("With 𝜎 (68%% probability) time is %f ± %f ms\n", avg, dev);
	printf("With 3𝜎 (99.73%% probability) time is %f ± %f ms\n", avg, dev*3);

	float max_err = 0, max_outlier = 0;
	int num_outliers = 0;
	for (int i = 0; i < num_runs; ++i) {
		float rel_err = fabs(times[num_startup + i] - avg) / dev;
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
}

int main(int argc, char * const *argv)
{
	int num_runs = -1;
	mandelbrot = mandelbrot_simple;
	int num_startup = 4;

	int opt = 0;
	while ((opt = getopt(argc, argv, "o:b:s:")) != -1) {
		switch (opt) {
		case 'o': {
			int var = atoi(optarg);
			switch(var) {
			case 0:
				mandelbrot = mandelbrot_simple;
				break;
			case 1:
				mandelbrot = mandelbrot_avx;
				break;
			case 2:
				mandelbrot = mandelbrot_avx2;
				break;
			default:
				printf("Unknown optimization option `-o %d`!\n", var);
				return 1;
			}
			mandelbrot = mandelbrot_avx2;
			break;
		}
		case 'b':
			num_runs = atoi(optarg);
			break;
		case 's':
			num_startup = atoi(optarg);
			break;
		default:
			// getopt prints the error
			return 1;
		}
	}

	if (num_runs > 0) {
		do_benchmark(num_runs, num_startup);
		return 0;
	}

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        DIE("Failed to init SDL: %s", SDL_GetError());

    SDL_Window *win = SDL_CreateWindow(
        "A window",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DFLT_WIDTH, DFLT_HEIGHT, SDL_WINDOW_SHOWN
    );
    if (!win)
        DIE("Failed to create a window: %s", SDL_GetError());

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *framebuffer = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, DFLT_WIDTH, DFLT_HEIGHT
    );

    ARGB *fbdata = (ARGB*) calloc(DFLT_WIDTH * DFLT_HEIGHT, sizeof(ARGB));
	struct State *state = calloc(1, sizeof(*state));
	state->fb = fbdata;

	state->w = state->gdata.bwidth = DFLT_WIDTH;
	state->h = state->gdata.bheight = DFLT_HEIGHT;

	state->gdata.exit_steps = aligned_alloc(
			32,
			state->gdata.bwidth * state->gdata.bheight * sizeof(*state->gdata.exit_steps)
	);
	state->gdata.exit_steps_alt = aligned_alloc(
			32,
			state->gdata.bwidth * state->gdata.bheight * sizeof(*state->gdata.exit_steps_alt)
	);
	printf("alloced %p and %p\n", state->gdata.exit_steps, state->gdata.exit_steps_alt);
	fflush(stdout);
	state->gdata.max_steps = 255;
	
	state->gdata.nxc = 0;
	state->gdata.nyc = 0;
	state->gdata.nswidth = 2;

	pthread_mutex_init(&push_mutex, NULL);

	pthread_t render;
	pthread_create(&render, 0, (void*(*)(void*))render_thread, state);

	char msgbuf[256];
    SDL_Event evt;
    while (!shall_quit) {
        while (SDL_PollEvent(&evt) != 0) {
            switch (evt.type) {
            case SDL_QUIT:
                shall_quit = true;
                break;
			case SDL_KEYDOWN:
				switch (evt.key.keysym.sym) {
				case SDLK_UP:
					state->gdata.nyc -= state->gdata.swidth * ARROW_STEP_DIST;
					break;
				case SDLK_DOWN:
					state->gdata.nyc += state->gdata.swidth * ARROW_STEP_DIST;
					break;
				case SDLK_LEFT:
					state->gdata.nxc -= state->gdata.swidth * ARROW_STEP_DIST;
					break;
				case SDLK_RIGHT:
					state->gdata.nxc += state->gdata.swidth * ARROW_STEP_DIST;
					break;
				case SDLK_PAGEUP:
					state->gdata.nswidth *= 1.5;
					break;
				case SDLK_PAGEDOWN:
					state->gdata.nswidth /= 1.5;
					break;
				}
				break;
            }
        }

		pthread_mutex_lock(&push_mutex);
		for (int i = 0; i < state->h; ++i)
			for (int j = 0; j < state->gdata.bwidth; ++j)
				state->fb[i * state->w + j] = shade(state->gdata.exit_steps_alt[i*state->gdata.bwidth + j]);
		pthread_mutex_unlock(&push_mutex);

		snprintf(msgbuf, sizeof(msgbuf)-1, "Render time: %9.3f ms / %6.2f FPS", render_time, 1000 / render_time);
		ui_puts(state, 10, 10, RGB(255, 255, 255), msgbuf);
		snprintf(msgbuf, sizeof(msgbuf)-1, "X %9.7f Y %9.7f S %9.7f", state->gdata.xc, state->gdata.yc, state->gdata.swidth);
		ui_puts(state, 10, 10 + FONT_SIZE_Y + 5, RGB(255, 255, 255), msgbuf);

        SDL_UpdateTexture(framebuffer, NULL, fbdata, DFLT_WIDTH * sizeof(ARGB));
        SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~ 60 fps
    }

	pthread_join(render, NULL);
	pthread_mutex_destroy(&push_mutex);
	free(state->gdata.exit_steps);
	free(state->gdata.exit_steps_alt);

    return 0;
}
