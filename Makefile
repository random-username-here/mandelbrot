
CC       := gcc
CFLAGS   := -Wall -g -O2 -mavx2 -Isrc/
LDFLAGS  := -g -lSDL2 -lm

COMMON_SOURCES := $(wildcard src/color/*.c src/gen/*.c)
BENCH_SOURCES := $(wildcard src/benchmark/*.c)
VIEWER_SOURCES := $(wildcard src/viewer/*.c)

COMMON_OBJS := $(patsubst %.c, %.o, $(COMMON_SOURCES))
VIEWER_OBJS := $(patsubst %.c, %.o, $(VIEWER_SOURCES))
BENCH_OBJS  := $(patsubst %.c, %.o, $(BENCH_SOURCES))

all: viewer

viewer: $(COMMON_OBJS) $(VIEWER_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

benchmark: $(COMMON_OBJS) $(BENCH_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(COMMON_OBJS) $(VIEWER_OBJS) $(BENCH_OBJS) viewer
