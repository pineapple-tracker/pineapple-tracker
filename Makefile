CC = clang
CFLAGS = -g -std=c99 -O2 -Wall -Wno-comment $(SDL_CFLAGS) \
	$(NCURSES_CFLAGS)

LIBS = $(shell sdl2-config --libs) \
	$(NCURSES_LIBS) \
	$(shell pkg-config --libs caca)

SDL_CFLAGS = $(shell sdl2-config --cflags)
NCURSES_CFLAGS = \
	$(shell ncurses5.4-config --cflags || ncurses5-config --cflags)
NCURSES_LIBS = \
	$(shell ncurses5.4-config --libs || ncurses5-config --libs)

all: pt

pt: main.o pt.o lft.o gui.o modes.o actions.o drivers/sdl/sdl_audio.o hvl_replay.o
	$(CC) -o $@ $^ ${LIBS}

player:	player.o lft.o gui.o modes.o actions.o
	$(CC) -o $@ $^ ${LIBS}

sdl_gui: sdl_gui.o pt.o gui.o modes.o actions.o lft.o
	$(CC) -o $@ $^ ${LIBS}

.PHONY:
	clean
clean:	
	@echo "clean ..."
	@rm -f *.o pt player sdl_gui
