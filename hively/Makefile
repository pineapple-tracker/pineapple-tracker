CC = gcc
VERSION = alpha-mega
CFLAGS = -std=c99 -O2 -Wall $(SDL_CFLAGS) $(NCURSES_CFLAGS)

SDL_CFLAGS := $(shell pkg-config --cflags sdl)
NCURSES_CFLAGS := $(shell ncurses5-config --cflags)

LIBS := -O2 $(shell pkg-config --libs sdl) \
		$(shell ncurses5-config --libs)

all: pineapple-tracker

pineapple-tracker:	main.o gui.o hvl_replay.o modes.o actions.o
	$(CC) -o $@ $^ ${LIBS}

old: main.o gui-old.o hvl_replay.o
	$(CC) -o $@ $^ ${LIBS}

%.o:	%.c types.h hvl_replay.h Makefile

.PHONY:
	clean
clean:	
	@echo "clean ..."
	@rm -f *.o
