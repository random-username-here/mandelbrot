#ifndef I_COMMON_H
#define I_COMMON_H

#define WIN_WIDTH      1024
#define WIN_HEIGHT     768

#define INITIAL_POS_X  0
#define INITIAL_POS_Y  0
#define INITIAL_SCALE  2

#define DIE(fmt, ...) \
	do {\
		fprintf(stderr, "Error: " fmt "\n" __VA_OPT__(,) __VA_ARGS__);\
		exit(EXIT_FAILURE);\
	} while (0)

#define SWAP(a, b) do { typeof(a) tmp = (a); (a) = (b); (b) = tmp; } while(0)

#define ARRAY_SIZE(arr) ((sizeof(arr)) / (sizeof(arr[0])))

#endif
