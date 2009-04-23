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

#include "stuff.h"

#define SETLO(v,x) v = ((v) & 0xf0) | (x)
#define SETHI(v,x) v = ((v) & 0x0f) | ((x) << 4)
#define CTRL(c) ((c) & 037)
#define KEY_ESCAPE 27
#define KEY_TAB 9   // this also happens to be ^i...
#define ENTER 13

#ifndef WINDOWS
#define BACKSPACE 0x107
#endif

/*                   */
// ** GLOBAL VARS ** //
/*                   */
int songx, songy, songoffs, songlen = 1;
int trackx, tracky, trackoffs, tracklen = TRACKLEN;
int instrx, instry, instroffs;

// 0 is like a blank command
char *validcmds = "0dfi@smtvw~+=";

/*char *keymap[2] = {
	";oqejkixdbhmwnvsz",
	"'2,3.p5y6f7gc9r0l/="
};*/

char *keymap[2] = {
	"zsxdcvgbhnjm,l.;/",
	"q2w3er5t6y7ui9o0p"
};

struct instrument instrument[256], iclip[256];
struct track track[256], tclip[256];
struct songline song[256];

/*                  */
// ** LOCAL VARS ** //
/*                  */
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

static int tcliplen, icliplen = 0;

static int currmode = PM_NORMAL;

static char filename[1024];

static char *notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-"};

/*                       */
// ** LOCAL FUNCTIONS ** //
/*                       */
static void _drawgui(void);
static void _initsonglines(void);
static void _inittracks(void);
static void _initinstrs(void);
static int _hexdigit(char c);
static int _hexinc(int x);
static int _hexdec(int x);
static int _nextfreetrack(void);
static int _nextfreeinstr(void);
static char _nextchar(void);
static int _char2int(char ch);
static int _freqkey(int c);
static void _display(void);
void _setdisplay(char *str);
