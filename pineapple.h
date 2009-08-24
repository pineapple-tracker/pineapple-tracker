#include <stdint.h>

#ifndef PINEAPPLE_H
#define PINEAPPLE_H
#define TRACKLEN 32

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

u8 callbacktime;
char filename[1024];
char *infinitemsg;
char comment[1024];

/* MODES */
enum {
	PM_NORMAL,
	PM_VISUAL,
	PM_VISUALLINE,
	PM_CMDLINE,
	PM_INSERT,
	PM_JAMMER
};

void normalmode(int c);
void cmdlinemode(void);
void insertmode(void);
void jammermode(void);
void visualmode(void);
void visuallinemode(void);

typedef enum {
	WF_TRI = 0,
	WF_SAW,
	WF_PUL,
	WF_NOI,
	WF_SINE
} waveform_t;

volatile struct oscillator {
	u16	freq;
	u16	phase;
	u16	duty;
	waveform_t waveform;
	u8 volume;	// 0-255
} osc[4];

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

struct instrument instrument[256], iclip[256];
//struct track track[256], tclip[256];
struct track tclip[256];
//struct songline song[256];

//int songlen;
int tracklen;

void initchip(void);
u8 interrupthandler(void);

void readsong(int pos, int ch, u8 *dest);
void readtrack(int num, int pos, struct trackline *tl);
void readinstr(int num, int pos, u8 *il);

void silence(void);
void lft_iedplonk(int, int);

void initgui(void);
void eventloop(void);

void display(void);

int hexdigit(char c);
int nextfreetrack(void);
int nextfreeinstr(void);

void startplaysong(int);
void startplaytrack(int);

void parsecmd(char cmd[]);

// just some poorly-named variables for hackin together the repeat command
int cmdrepeat;
int cmdrepeatnum;
int lastaction;
int lastrepeatnum;

/* lines to be highlighted in visual line mode */
int highlight_firstline, highlight_lastline, highlight_lineamount;

/* lines to be highlighted in visual mode */
int highlight_firstx, highlight_firsty, highlight_lasty, highlight_lastx; 

/* ACTIONS */
void act_bigmvdown(void);
void act_bigmvup(void);
void act_mvbottom(void);
void act_mvtop(void);
void act_clritall(void);
void act_clronething(void);
void act_clrinsongtab(int y);
void act_clrintracktab(int t, int y);
void act_clrininstrtab(int instr, int y);
void act_fxdec(void);
void act_fxinc(void);
void act_instrdec(void);
void act_instrinc(void);
void act_mvdown(void);
void act_mvleft(void);
void act_mvright(void);
void act_mvup(void);
void act_notedec(void);
void act_noteinc(void);
void act_octavedec(void);
void act_octaveinc(void);
void act_paramdec(void);
void act_paraminc(void);
void act_trackdec(void);
void act_trackinc(void);
void act_transpdec(void);
void act_transpinc(void);
void act_undo(void);
void act_viewinstrdec(void);
void act_viewinstrinc(void);
void act_viewtrackdec(void);
void act_viewtrackinc(void);
void act_addline(void);
void act_delline(void);

u8 trackpos;
/*u8 songpos;
int songlen;*/
int playtrack;
int playsong;

struct config_params {
	int buffersize;
	char *gui;
	char *audio_driver;
	int samplefreq;
};

/*audio functions*/

void lftSdlCallback(void *userdata, u8 *buf, int len);
u8 lft_sdl_init(void);
//u8 (*audioInit)(void); //function that changes depending on which audiodriver is specified
//u8 sdl_init(void);
void (*sdlCallback)(void); //function that changes depending on which type of file is loaded
//void hvlSdlCallBack(struct hvl_tune *ht, uint8 *stream, int length);

enum {
	LFT,
	AHX
};

typedef struct _pineapple_tune {
	int type; // LFT, AHX

	char *filename;
	u8 callbacktime; //ht_SpeedMultiplier
	u16 tempo; //ht_Tempo
	char *comment;
	struct oscillator osc[4];
	struct instrument instr[256]; //struct hvl_instrument *ht_Instruments
	struct track trk[256]; //struct hvl_step ht_Tracks[256][64]
	struct songline sng[256]; //struct hvl_position *ht_Positions
	int songlen; //ht_PositionNr
	int songpos; //ht_PosNr
	int trackpos;
	int tracklen; //ht_TrackLength
	int trackNum; //number of tracks that are used, ht_TrackNr
	int instrx, instry, instroffs;
	int songx, songy, songoffs;
	int trackx, tracky, trackoffs;
	int currtrack, currinstr;
	int currtab;
	int saved;

	void (*iedplonk)(int note, int instr, ...);
} pineapple_tune;

pineapple_tune *tune;

u8 hvl_sdl_init(void);

#endif /* PINEAPPLE_H */
