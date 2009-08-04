CC = gcc
VERSION = alpha-mega
CFLAGS = -g -std=c99 -O2 -Wall $(SDL_CFLAGS) $(NCURSES_CFLAGS)

SDL_CFLAGS := $(shell pkg-config --cflags sdl)
NCURSES_CFLAGS := $(shell ncurses5-config --cflags)

LIBS := -O2 $(shell pkg-config --libs sdl) \
		$(shell ncurses5-config --libs)

all: pineapple-hively

pineapple-hively: main.o gui.o hvl_replay.o actions.o
	$(CC) -o $@ $^ ${LIBS}

%.o: %.c pineapple.h gui.h hvl_replay.h Makefile

.PHONY:
	clean
clean:	
	@echo "clean ..."
	@rm -f *.o pineapple-hively
