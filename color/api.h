#ifndef I_COLOR_API
#define I_COLOR_API

#include <stdint.h>

struct __attribute__((packed)) ARGB {
	uint8_t b, g, r, a;
};

typedef struct ARGB ARGB;

#define RGB(_r, _g, _b) ((ARGB) { .r = (_r), .g = (_g), .b = (_b) })

#endif
