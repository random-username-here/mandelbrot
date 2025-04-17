///
/// All commonly used things by the renderer
///
#ifndef I_VIEWER
#define I_VIEWER

#include "common.h"
#include "color/api.h"
#include "gen/api.h"
#include <pthread.h>
#include <stdbool.h>

struct State {
	ARGB *fb;
	int *exit_steps_rendered;
	int *exit_steps_ready;
	struct Mb_GeneratorData gdata;
	struct {
		float xc, yc, swidth;
	} new_params;
	bool shall_quit;
	int generator, colorizer;
	pthread_mutex_t data_mutex;
	float ms_per_frame;
	bool has_fresh_data;
};

// Graphical routines

#define C_WHITE RGB(255, 255, 255)
#define C_GRAY RGB(200, 200, 200)
#define C_DARKER_GRAY RGB(150, 150, 150)

void ui_fillrect(struct State *st, int x, int y, int w, int h, ARGB color);

struct UI_TextFlow {
	struct State *state;
	int x, y, base_x;
};

void ui_textflow_init(struct UI_TextFlow *flow, struct State *state, int x, int y);

void ui_textflow_puts(struct UI_TextFlow *flow, ARGB color, const char *text);

__attribute__((format(printf, 3, 4)))
void ui_textflow_printf(struct UI_TextFlow *flow, ARGB color, const char *fmt, ...);

#endif
