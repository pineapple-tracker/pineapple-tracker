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

struct instrument instrument[256], iclip[256];
struct track track[256], tclip[256];
struct songline song[256];

/*                  */
// ** LOCAL VARS ** //
/*                  */
/*
static int currtrack = 1, currinstr = 1;
static int currtab = 0;
static int octave = 4;
static char cmdstr[50] = "";
static char *dispmesg = "";
static int disptick = 0;
static int cmdrepeat = 0;
static int cmdrepeatnum = 1;
static int lastrepeat = 1;
static int lastaction;
static int f;
static int saved = 1;


static int currmode = PM_NORMAL;

static char filename[1024];
*/

void act_viewinstrdec(void);
void act_viewinstrinc(void);
void drawgui(void);
int freqkey(int c);

