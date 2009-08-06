CC = gcc
CFLAGS = -g -std=c99 -O2 -Wall -Wno-comment $(SDL_CFLAGS) \
		$(NCURSES_CFLAGS)

SDL_CFLAGS = $(shell pkg-config --cflags sdl)
NCURSES_CFLAGS = $(shell ncurses5-config --cflags)

LIBS = $(shell pkg-config --libs sdl) \
	$(shell ncurses5-config --libs) \
	$(shell pkg-config --libs caca) \
	#$(shell pkg-config --libs jack)

all: pineapple-tracker player

pineapple-tracker: main.o oldchip.o gui.o modes.o actions.o musicchip_file.o \
		conf_file.o
	$(CC) -o $@ $^ ${LIBS}

player:	player.o oldchip.o gui.o modes.o actions.o musicchip_file.o conf_file.o
	$(CC) -o $@ $^ ${LIBS}

sdl_gui: sdl_gui.o gui.o modes.o actions.o oldchip.o musicchip_file.o
	$(CC) -o $@ $^ -O2 ${SDL_CFLAGS} -lSDLmain -lSDL

.PHONY:
	clean
clean:	
	@echo "clean ..."
	@rm -f *.o pineapple-tracker player sdl_gui
