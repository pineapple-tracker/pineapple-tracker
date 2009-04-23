/* vi:set ts=4 sts=4 sw=4: */
#include <stdint.h>

#ifndef STUFF_H
#define STUFF_H
#define TRACKLEN 32

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

u8 callbacktime;

enum {
	WF_TRI,
	WF_SAW,
	WF_PUL,
	WF_NOI,
	WF_SINE
};

/* pineapple modes */
enum {
	PM_NORMAL,
	PM_CMDLINE,
	PM_INSERT,
	PM_JAMMER
};

struct trackline {
	u8	note;
	u8	instr;
	u8	cmd[2];
	u8	param[2];
};

struct track {
	struct trackline	line[TRACKLEN];
};

struct instrline {
	u8			cmd;
	u8			param;
};

struct instrument {
	int			length;
	struct instrline	line[256];
};

struct songline {
	u8			track[4];
	u8			transp[4];
};

volatile struct oscillator {
	u16	freq;
	u16	phase;
	u16	duty;
	u8	waveform;
	u8	volume;	// 0-255
} osc[4];

void initchip();
u8 interrupthandler();

void readsong(int pos, int ch, u8 *dest);
void readtrack(int num, int pos, struct trackline *tl);
void readinstr(int num, int pos, u8 *il);

void silence();
void iedplonk(int, int);

void initgui();
void guiloop();

//void initjoystick();
//void sdlmainloop();
void display();

void startplaysong(int);
void startplaytrack(int);
int loadfile(char *);

void parsecmd(char cmd[]);

void jammermode(void);

extern u8 trackpos;
extern u8 playtrack;
extern u8 playsong;
extern u8 songpos;
extern int songlen;

extern struct instrument instrument[256], iclip;
extern struct track track[256], tclip;
extern struct songline song[256];

#endif /* STUFF_H */
