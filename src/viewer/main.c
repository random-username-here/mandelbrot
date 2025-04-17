#include "color/api.h"
#include "common.h"
#include "gen/api.h"
#include "viewer.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define ALIGN 32
#define MAX_STEPS 256

#define MOVE_STEP 0.2
#define SCALE_STEP 1.5

static void init_state(struct State *state)
{
	state->fb = calloc(WIN_WIDTH * WIN_HEIGHT, sizeof(*state->fb));
	state->exit_steps_rendered = aligned_alloc(
			ALIGN,
			WIN_WIDTH * WIN_HEIGHT * sizeof(*state->exit_steps_rendered)
	);
	state->exit_steps_ready = aligned_alloc(
			ALIGN,
			WIN_WIDTH * WIN_HEIGHT * sizeof(*state->exit_steps_ready)
	);
	state->gdata.exit_steps = aligned_alloc(
			ALIGN,
			WIN_WIDTH * WIN_HEIGHT * sizeof(*state->gdata.exit_steps)
	);
	state->gdata.max_steps = MAX_STEPS;
	state->new_params.xc = state->gdata.xc = INITIAL_POS_X;
	state->new_params.yc = state->gdata.yc = INITIAL_POS_Y;
	state->new_params.swidth = state->gdata.swidth = INITIAL_SCALE;
	state->gdata.bheight = WIN_HEIGHT;
	state->gdata.bwidth = WIN_WIDTH;
	state->shall_quit = false;
	state->has_fresh_data = false;
	state->ms_per_frame = INFINITY;

	state->generator = DEFAULT_GENERATOR;
	state->colorizer = DEFAULT_COLORIZER;

	pthread_mutex_init(&state->data_mutex, NULL);
}

static void deinit_state(struct State *state)
{
	free(state->fb);
	free(state->exit_steps_ready);
	free(state->exit_steps_rendered);
	free(state->gdata.exit_steps);
	pthread_mutex_destroy(&state->data_mutex);
}

static void generator_main(struct State *state)
{
	void (*generator)(struct Mb_GeneratorData *gdata)
		= generators[state->generator].mandelbrot;

	while (!state->shall_quit) {

		// Compute
		// gdata is only for this thread
		
		clock_t begin = clock();
		generator(&state->gdata);
		clock_t end = clock();
		
		pthread_mutex_lock(&state->data_mutex);

		// Here we can safely access shared state

		// Push updates
		SWAP(state->gdata.exit_steps, state->exit_steps_ready);
		state->has_fresh_data = true;
		state->ms_per_frame = (end - begin) * 1.0f / CLOCKS_PER_SEC * 1000;

		// Load new params
		state->gdata.xc = state->new_params.xc;
		state->gdata.yc = state->new_params.yc;
		state->gdata.swidth = state->new_params.swidth;

		pthread_mutex_unlock(&state->data_mutex);

		pthread_testcancel();
	}
}

static void draw_ui(struct State *state)
{
	ARGB (*colorizer)(int steps, int max_steps)
		= colorizers[state->colorizer].color;


	// Load step counts
	pthread_mutex_lock(&state->data_mutex);
	if (state->has_fresh_data) {
		SWAP(state->exit_steps_ready, state->exit_steps_rendered);
		state->has_fresh_data = false;
	}
	float ms_per_frame = state->ms_per_frame;
	pthread_mutex_unlock(&state->data_mutex);

	// Paint the image
	for (int i = 0; i < WIN_HEIGHT; ++i)
		for (int j = 0; j < WIN_WIDTH; ++j)
			state->fb[i * WIN_WIDTH + j]
					= colorizer(
							state->exit_steps_rendered[i * WIN_WIDTH + j],
							MAX_STEPS
					);

	// Draw text gui
	
	struct UI_TextFlow flow;
	ui_textflow_init(&flow, state, 20, 20);
	ui_textflow_puts(&flow, C_WHITE, "Mandelbrot set visualizer\n\n");
	ui_textflow_printf(&flow, C_WHITE, "%-5.2f ms", ms_per_frame);
	ui_textflow_puts(&flow, C_GRAY, " per frame, ");
	ui_textflow_printf(&flow, C_WHITE, "%-5.2f fps\n", 1000 / ms_per_frame);
	ui_textflow_printf(
			&flow, C_GRAY, "X %-5.3f Y %-5.3f S %-5.3f\n",
			state->gdata.xc, state->gdata.yc, state->gdata.swidth
	);
	ui_textflow_puts(&flow, C_GRAY, "Generator: ");
	ui_textflow_puts(&flow, C_WHITE, generators[state->generator].name);
	ui_textflow_puts(&flow, C_DARKER_GRAY, " [g]");
	ui_textflow_puts(&flow, C_GRAY, "\nColorizer: ");
	ui_textflow_puts(&flow, C_WHITE, colorizers[state->colorizer].name);
	ui_textflow_puts(&flow, C_DARKER_GRAY, " [c]\n");
	ui_textflow_puts(&flow, C_DARKER_GRAY, "Arrows to move, [PgUp]/[PgDn] to zoom");

}

void handle_key(struct State *state, SDL_Keycode key, bool *restart)
{
	switch(key) {

	case SDLK_UP:
		state->new_params.yc -= state->gdata.swidth * MOVE_STEP;
		break;

	case SDLK_DOWN:
		state->new_params.yc += state->gdata.swidth * MOVE_STEP;
		break;

	case SDLK_LEFT:
		state->new_params.xc -= state->gdata.swidth * MOVE_STEP;
		break;

	case SDLK_RIGHT:
		state->new_params.xc += state->gdata.swidth * MOVE_STEP;
		break;

	case SDLK_PAGEUP:
		state->new_params.swidth *= SCALE_STEP;
		break;

	case SDLK_PAGEDOWN:
		state->new_params.swidth /= SCALE_STEP;
		break;

	case SDLK_g:
		state->generator = (state->generator+1) % ARRAY_SIZE(generators);
		*restart = true;
		break;
	
	case SDLK_c:
		state->colorizer = (state->colorizer+1) % ARRAY_SIZE(colorizers);
		break;

	}

}

int main(void)
{
	struct State state;
	init_state(&state);

	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0)
		DIE("Failed to init SDL: %s", SDL_GetError());

	SDL_Window *win = SDL_CreateWindow(
		"Mandelbrot visualizer",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN
	);

	if (!win)
        DIE("Failed to create a window: %s", SDL_GetError());

    SDL_Renderer *renderer
		= SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		DIE("Failed to init renderer: %s", SDL_GetError());

    SDL_Texture *framebuffer = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, WIN_WIDTH, WIN_HEIGHT
	);
	if (!framebuffer)
		DIE("Failed to init framebuffer: %s", SDL_GetError());
	
	pthread_t generator_thread;
	pthread_create(
		&generator_thread, 0, (void*(*)(void*)) generator_main, &state
	);

	bool will_quit = false;
	SDL_Event evt;
	while (!will_quit) {
		bool restart = false;
		while (SDL_PollEvent(&evt) != 0) {
			switch (evt.type) {
			case SDL_QUIT:
				will_quit = true;
				break;
			case SDL_KEYDOWN:
				handle_key(&state, evt.key.keysym.sym, &restart);
			}
		}

		draw_ui(&state);
        SDL_UpdateTexture(framebuffer, NULL, state.fb, WIN_WIDTH * sizeof(ARGB));
        SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
        SDL_RenderPresent(renderer);

		if (restart) {
			pthread_cancel(generator_thread);
			pthread_join(generator_thread, NULL);
			pthread_create(
				&generator_thread, 0, (void*(*)(void*)) generator_main, &state
			);
		}

        SDL_Delay(16); // ~ 60 fps
	}

	pthread_cancel(generator_thread);
	pthread_join(generator_thread, NULL);
	deinit_state(&state);

	return 0;
}
