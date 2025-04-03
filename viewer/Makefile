
CC		=	gcc
LIBS	= -lSDL2 -lm
CFLAGS 	= -Wall -g -O2 -mavx2
LDFLAGS	= -g

all: viewer

HDRS	= font.h viewer.h
OBJS 	= gfx.o window.o ../gen/avx2.o ../gen/simple.o ../gen/avx.o

$(OBJS): $(HDRS) Makefile

viewer: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LIBS)

clean:
	rm -rf $(OBJS)

run: viewer
	./viewer
