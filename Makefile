CC = gcc
VERSION = alpha-mega
CFLAGS = -g -std=c99 -O2 -pthread -Wall -Wno-comment $(SDL_CFLAGS) $(NCURSES_CFLAGS)

SDL_CFLAGS := $(shell pkg-config --cflags sdl)
NCURSES_CFLAGS := $(shell ncurses5-config --cflags)

LIBS := $(shell pkg-config --libs sdl) \
		$(shell ncurses5-config --libs) \
		$(shell pkg-config --libs caca) #\
		#$(shell pkg-config --libs jack)

all: pineapple-tracker player

pineapple-tracker:	main.o oldchip.o gui.o modes.o actions.o musicchip_file.c conf_file.c
	$(CC) -o $@ $^ ${LIBS}

player:		player.o oldchip.o gui.o modes.o actions.o musicchip_file.c conf_file.o
	$(CC) -o $@ $^ ${LIBS}

sdl_gui:
	$(CC) -o $@ sdl_gui.c -O2 ${SDL_CFLAGS} -lSDLmain -lSDL

%.o:	%.c pineapple.h gui.h musicchip_file.h conf_file.h Makefile

.PHONY:
	clean
clean:	
	@echo "clean ..."
	@rm -f *.o pineapple-tracker player sdl_gui
