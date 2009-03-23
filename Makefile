LDFLAGS = $(SDL_LDFLAGS) $(NCURSES_LDFLAGS)
CFLAGS = -Wall
CC = gcc

NCURSES_LDFLAGS := $(shell ncurses5-config --libs)
SDL_LDFLAGS := $(shell sdl-config --libs)

all:	tracker player

tracker:	main.o chip.o gui.o
	gcc -o $@ $^ ${LDFLAGS}

player:		player.o chip.o gui.o
	gcc -o $@ $^ ${LDFLAGS}

%.o:	%.c stuff.h Makefile

.PHONY:
	clean
clean:	
	@echo "clean ..."
	@rm -f *.o
