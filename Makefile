LDFLAGS=-lSDL -lncurses
CFLAGS=-Wall
CC=gcc

all:		tracker

tracker:	main.o chip.o gui.o
		gcc -o $@ $^ ${LDFLAGS}

%.o:		%.c stuff.h Makefile
