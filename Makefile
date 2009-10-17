CC = gcc
CFLAGS = -g -std=c99 -O2 -Wall -Wno-comment $(SDL_CFLAGS) \
		$(NCURSES_CFLAGS)

SDL_CFLAGS = $(shell pkg-config --cflags sdl)
NCURSES_CFLAGS = $(shell ncurses5-config --cflags)

LIBS = $(shell pkg-config --libs sdl) \
	$(shell ncurses5-config --libs) \
	#$(shell pkg-config --libs caca) \
	#$(shell pkg-config --libs jack)

all: pppt

pppt: main.o pt.o lft.o gui.o modes.o actions.o drivers/sdl/sdl_audio.o hvl_replay.o
	$(CC) -o $@ $^ ${LIBS}

player:	player.o lft.o gui.o modes.o actions.o
	$(CC) -o $@ $^ ${LIBS}

sdl_gui: sdl_gui.o pt.o gui.o modes.o actions.o lft.o
	$(CC) -o $@ $^ ${LIBS}

.PHONY:
	clean
clean:	
	@echo "clean ..."
	@rm -f *.o pppt player sdl_gui
