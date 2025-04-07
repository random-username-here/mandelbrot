///
/// All commonly used things by the renderer
///
#ifndef I_VIEWER
#define I_VIEWER

#define VIEWER

#include <SDL2/SDL_events.h>

#include "../gen/api.h"
#include "../color/api.h"
#include <pthread.h>

#define DFLT_WIDTH 1024
#define DFLT_HEIGHT 768
#define ARROW_STEP_DIST 0.1

#define DIE(fmt, ...) \
	do {\
		fprintf(stderr, "Error: " fmt "\n" __VA_OPT__(,) __VA_ARGS__);\
		exit(EXIT_FAILURE);\
	} while (0)

#define SWAP(a, b) do { typeof(a) tmp = (a); (a) = (b); (b) = tmp; } while(0)

struct State {

	// Set up by the windowing
	ARGB *fb;
	int w, h;

	struct Mb_GeneratorData gdata;

};

/// Graphical functions
void ui_puts(struct State *state, int x, int y, ARGB color, const char *text);
void ui_fillrect(struct State *state, int x, int y, int w, int h, ARGB color);

/// Benchmarks
struct Mb_BenchmarkState {
	int num_runs;
	float acceptable_var;
	void (*mandelbrot)(struct Mb_GeneratorData *gdata);
};

void do_benchmark(struct Mb_BenchmarkState *state);
#endif
