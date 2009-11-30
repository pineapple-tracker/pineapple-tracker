CC = gcc
CFLAGS = -g -std=c99 -Wall -lm $(SDL_CFLAGS)

SDL_CFLAGS = $(shell pkg-config --cflags sdl)

LIBS = $(shell pkg-config --libs sdl)

main: main.c
	$(CC) $(CFLAGS) -o $@ $^ ${LIBS}

.PHONY:
	clean
clean:
	@echo "clean..."
	@rm -f *.o main
