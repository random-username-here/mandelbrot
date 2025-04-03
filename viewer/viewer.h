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

#define MAX_GENERATORS 256

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

#endif
