/* vi:set ts=4 sts=4 sw=4: */
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
char currcmd;
int instrx, instry, instroffs;
int octave;
int songx, songy, songoffs, songlen;
int trackx, tracky, trackoffs, tracklen;
int currtrack, currinstr;
int currtab;
int saved;
int disptick;
int step;

int cmdrepeat;
int cmdrepeatnum;
int lastrepeat;
int lastaction;

char cmdstr[500];
char filename[1024];
char *validcmds;
char *keymap[2];

struct instrument instrument[256], iclip[256];
struct track track[256], tclip[256];
struct songline song[256];

void initsonglines(void);
void inittracks(void);
void initinstrs(void);

void drawgui(void);
int freqkey(int c);
int hexinc(int x);
int hexdec(int x);
void insertc (int c);
char nextchar(void);
void savefile(char *fname);
void setdisplay(char *str);
