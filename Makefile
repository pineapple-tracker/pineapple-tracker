CC = gcc
VERSION = alpha-mega
CFLAGS = -O2 -Wall $(SDL_CFLAGS) $(NCURSES_CFLAGS)

SDL_CFLAGS := $(shell pkg-config --cflags sdl)
NCURSES_CFLAGS := $(shell ncurses5-config --cflags)

LIBS := -O2 $(shell pkg-config --libs sdl) \
		$(shell ncurses5-config --libs) \
		$(shell pkg-config --libs caca) -lSDL

all: pineapple-tracker player

pineapple-tracker:	main.o chip.o gui.o
	$(CC) -o $@ $^ ${LIBS}

player:		player.o chip.o gui.o
	$(CC) -o $@ $^ ${LIBS}

%.o:	%.c stuff.h Makefile

.PHONY:
	clean
clean:	
	@echo "clean ..."
	@rm -f *.o
