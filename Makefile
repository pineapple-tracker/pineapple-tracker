CC = gcc
CFLAGS = -g -std=c99 -O2 -Wall -Wno-comment -fcommon $(SDL_CFLAGS) \
		$(NCURSES_CFLAGS)

SDL_CFLAGS = $(shell sdl2-config --cflags)

NCURSES_CFLAGS = $(shell pkg-config --libs ncurses)
NCURSES_LIBS = $(shell pkg-config --libs ncurses)

LIBS = $(shell sdl2-config --libs) \
	$(NCURSES_LIBS) \
	$(shell pkg-config --libs caca) \
	-lm

all: ptracker

ptracker: main.o pt.o lft.o gui.o modes.o actions.o drivers/sdl/sdl_audio.o hvl_replay.o
	$(CC) -o $@ $^ ${LIBS}

player:	player.o lft.o gui.o modes.o actions.o
	$(CC) -o $@ $^ ${LIBS}

sdl_gui: sdl_gui.o pt.o gui.o modes.o actions.o lft.o
	$(CC) -o $@ $^ ${LIBS}

.PHONY: all clean

clean:	
	@echo "clean ..."
	@rm -f *.o drivers/sdl/*.o ptracker player sdl_gui
