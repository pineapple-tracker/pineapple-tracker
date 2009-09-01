CC = gcc
CFLAGS = -std=c99 -Wall $(SDL_CFLAGS)

SDL_CFLAGS = $(shell pkg-config --cflags sdl)

LIBS = $(shell pkg-config --libs sdl)

main: main.c
	$(CC) -o $@ $^ ${LIBS}
