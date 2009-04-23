#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curses.h>
#include <unistd.h>
#include <ctype.h>

#ifndef WINDOWS
#include <err.h>
#endif

#define SETLO(v,x) v = ((v) & 0xf0) | (x)
#define SETHI(v,x) v = ((v) & 0x0f) | ((x) << 4)
#define CTRL(c) ((c) & 037)
#define KEY_ESCAPE 27
#define KEY_TAB 9   // this also happens to be ^i...
#define ENTER 13

#ifndef WINDOWS
#define BACKSPACE 0x07
#endif

int currmode;
int instrx, instry, instroffs;
int octave;
int songx, songy, songoffs, songlen;
int trackx, tracky, trackoffs, tracklen;
int currtrack, currinstr;
int currtab;

char *validcmds;
char *keymap[2];

struct instrument instrument[256], iclip;
struct track track[256], tclip;
struct songline song[256];


void act_viewinstrdec(void);
void act_viewinstrinc(void);
void drawgui(void);
int freqkey(int c);

