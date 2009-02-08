#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>
#include <err.h>
#include <SDL/SDL.h>

#include "stuff.h"

#define SETLO(v,x) v = ((v) & 0xf0) | (x)
#define SETHI(v,x) v = ((v) & 0x0f) | ((x) << 4)
#define CTRL(c) ((c) & 037)
#define KEY_ESCAPE 27
#define KEY_TAB 9   // this also happens to be ^i...


int songx, songy, songoffs, songlen = 1;
int trackx, tracky, trackoffs, tracklen = TRACKLEN;
int instrx, instry, instroffs;
int currtrack = 1, currinstr = 1;
int currtab = 0;
int octave = 4;
bool vimode = false;
bool insertmode = false;
bool cmdmode = false;
char *cmdstr = "";
char *dispmesg = "";
int disptick = 0;
bool sdl_finished = false;
int currbutt = -1;
int winheight = 0;
void drawgui();
bool cmdrepeat = false;
int cmdrepeatnum = 1;

char filename[1024];

char *notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-"};

char *validcmds = "0dfijlmtvw~+=";

/*char *keymap[2] = {
	";oqejkixdbhmwnvsz",
	"'2,3.p5y6f7gc9r0l/="
};*/

char *keymap[2] = {
	"zsxdcvgbhnjm,l.;/",
	"q2w3er5t6y7ui9o0p"
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

struct instrument instrument[256], iclip;
struct track track[256], tclip;
struct songline song[256];

enum {
	PM_IDLE,
	PM_PLAY,
	PM_EDIT
};
int playmode = PM_IDLE;

int hexdigit(char c) {
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

/* these wrap around */
int hexinc(int x) {
	return (x >= 0 && x <= 14)? x+1 : 0;
}
int hexdec(int x) {
	return (x >= 1 && x <= 15)? x-1 : 15;
}

int freqkey(int c) {
	char *s;
	int f = -1;

	if(c == '-' || c == KEY_DC) return 0;
	if(c > 0 && c < 256) {
		s = strchr(keymap[0], c);
		if(s) {
			f = (s - (keymap[0])) + octave * 12 + 1;
		} else {
			s = strchr(keymap[1], c);
			if(s) {
				f = (s - (keymap[1])) + octave * 12 + 12 + 1;
			}
		}
	}
	if(f > 12 * 9 + 1) return -1;
	return f;
}

void readsong(int pos, int ch, u8 *dest) {
	dest[0] = song[pos].track[ch];
	dest[1] = song[pos].transp[ch];
}

void readtrack(int num, int pos, struct trackline *tl) {
	tl->note = track[num].line[pos].note;
	tl->instr = track[num].line[pos].instr;
	tl->cmd[0] = track[num].line[pos].cmd[0];
	tl->cmd[1] = track[num].line[pos].cmd[1];
	tl->param[0] = track[num].line[pos].param[0];
	tl->param[1] = track[num].line[pos].param[1];
}

void readinstr(int num, int pos, u8 *il) {
	if(pos >= instrument[num].length) {
		il[0] = 0;
		il[1] = 0;
	} else {
		il[0] = instrument[num].line[pos].cmd;
		il[1] = instrument[num].line[pos].param;
	}
}

void savefile(char *fname) {
	FILE *f;
	int i, j;

	f = fopen(fname, "w");
	if(!f) {
		fprintf(stderr, "save error!\n");
		return;
	}

	fprintf(f, "musicchip tune\n");
	fprintf(f, "version 1\n");
	fprintf(f, "\n");
	for(i = 0; i < songlen; i++) {
		fprintf(f, "songline %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i,
			song[i].track[0],
			song[i].transp[0],
			song[i].track[1],
			song[i].transp[1],
			song[i].track[2],
			song[i].transp[2],
			song[i].track[3],
			song[i].transp[3]);
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++) {
		for(j = 0; j < tracklen; j++) {
			struct trackline *tl = &track[i].line[j];

			if(tl->note || tl->instr || tl->cmd[0] || tl->cmd[1]) {
				fprintf(f, "trackline %02x %02x %02x %02x %02x %02x %02x %02x\n",
					i,
					j,
					tl->note,
					tl->instr,
					tl->cmd[0],
					tl->param[0],
					tl->cmd[1],
					tl->param[1]);
			}
		}
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++) {
		if(instrument[i].length > 1) {
			for(j = 0; j < instrument[i].length; j++) {
				fprintf(f, "instrumentline %02x %02x %02x %02x\n",
					i,
					j,
					instrument[i].line[j].cmd,
					instrument[i].line[j].param);
			}
		}
	}

	fclose(f);
}

void loadfile(char *fname) {
	FILE *f;
	char buf[1024];
	int cmd[3];
	int i1, i2, trk[4], transp[4], param[3], note, instr;
	int i;

	snprintf(filename, sizeof(filename), "%s", fname);

	f = fopen(fname, "r");
	if(!f) {
		return;
	}

	songlen = 1;
	while(!feof(f) && fgets(buf, sizeof(buf), f)) {
		if(9 == sscanf(buf, "songline %x %x %x %x %x %x %x %x %x",
			&i1,
			&trk[0],
			&transp[0],
			&trk[1],
			&transp[1],
			&trk[2],
			&transp[2],
			&trk[3],
			&transp[3])) {

			for(i = 0; i < 4; i++) {
				song[i1].track[i] = trk[i];
				song[i1].transp[i] = transp[i];
			}
			if(songlen <= i1) songlen = i1 + 1;
		} else if(8 == sscanf(buf, "trackline %x %x %x %x %x %x %x %x",
			&i1,
			&i2,
			&note,
			&instr,
			&cmd[0],
			&param[0],
			&cmd[1],
			&param[1])) {

			track[i1].line[i2].note = note;
			track[i1].line[i2].instr = instr;
			for(i = 0; i < 2; i++) {
				track[i1].line[i2].cmd[i] = cmd[i];
				track[i1].line[i2].param[i] = param[i];
			}
		} else if(4 == sscanf(buf, "instrumentline %x %x %x %x",
			&i1,
			&i2,
			&cmd[0],
			&param[0])) {

			instrument[i1].line[i2].cmd = cmd[0];
			instrument[i1].line[i2].param = param[0];
			if(instrument[i1].length <= i2) instrument[i1].length = i2 + 1;
		}
	}

	fclose(f);
}

void exitgui() {
	endwin();
}

void initgui() {
	int i;

	initscr();
	// overrides your terminal's bg color :(
	//start_color();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);

	// this makes screen update when you're not pressing keys
	nodelay(stdscr, TRUE);

	//raw();
	//halfdelay(1);
	//curs_set(2);

	for(i = 1; i < 256; i++) {
		instrument[i].length = 1;
		instrument[i].line[0].cmd = '0';
		instrument[i].line[0].param = 0;
	}

	atexit(exitgui);
}

void drawsonged(int x, int y, int height) {
	int i, j;
	char buf[1024];

	if(songy < songoffs) songoffs = songy;
	if(songy >= songoffs + height) songoffs = songy - height + 1;

	for(i = 0; i < songlen; i++) {
		if(i >= songoffs && i - songoffs < height) {
			move(y + i - songoffs, x + 0);
			if(i == songy) attrset(A_BOLD);
			snprintf(buf, sizeof(buf), "%02x: ", i);
			addstr(buf);
			for(j = 0; j < 4; j++) {
				snprintf(buf, sizeof(buf), "%02x:%02x", song[i].track[j], song[i].transp[j]);
				addstr(buf);
				if(j != 3) addch(' ');
			}
			if(playsong && songpos == (i + 1)) {
				attrset(A_STANDOUT);
				addch('*');
			}
			attrset(A_NORMAL);
		}
	}
}

void drawtracked(int x, int y, int height) {
	int i, j;
	char buf[1024];

	if(tracky < trackoffs) trackoffs = tracky;
	if(tracky >= trackoffs + height) trackoffs = tracky - height + 1;

	for(i = 0; i < tracklen; i++) {
		if(i >= trackoffs && i - trackoffs < height) {
			move(y + i - trackoffs, x + 0);
			if(i == tracky) attrset(A_BOLD);
			snprintf(buf, sizeof(buf), "%02x: ", i);
			addstr(buf);
			if(track[currtrack].line[i].note) {
				snprintf(buf, sizeof(buf), "%s%d",
					notenames[(track[currtrack].line[i].note - 1) % 12],
					(track[currtrack].line[i].note - 1) / 12);
			} else {
				snprintf(buf, sizeof(buf), "---");
			}
			addstr(buf);
			snprintf(buf, sizeof(buf), " %02x", track[currtrack].line[i].instr);
			addstr(buf);
			for(j = 0; j < 2; j++) {
				if(track[currtrack].line[i].cmd[j]) {
					snprintf(buf, sizeof(buf), " %c%02x",
						track[currtrack].line[i].cmd[j],
						track[currtrack].line[i].param[j]);
				} else {
					snprintf(buf, sizeof(buf), " ...");
				}
				addstr(buf);
			}
			if(playtrack && ((i + 1) % tracklen) == trackpos) {
				attrset(A_STANDOUT);
				addch('*');
			}
			attrset(A_NORMAL);
		}
	}
}

void drawinstred(int x, int y, int height) {
	int i;
	char buf[1024];

	if(instry >= instrument[currinstr].length) instry = instrument[currinstr].length - 1;

	if(instry < instroffs) instroffs = instry;
	if(instry >= instroffs + height) instroffs = instry - height + 1;

	for(i = 0; i < instrument[currinstr].length; i++) {
		if(i >= instroffs && i - instroffs < height) {
			move(y + i - instroffs, x + 0);
			if(i == instry) attrset(A_BOLD);
			snprintf(buf, sizeof(buf), "%02x: %c ", i, instrument[currinstr].line[i].cmd);
			addstr(buf);
			if(instrument[currinstr].line[i].cmd == '+' || instrument[currinstr].line[i].cmd == '=') {
				if(instrument[currinstr].line[i].param) {
					snprintf(buf, sizeof(buf), "%s%d",
						notenames[(instrument[currinstr].line[i].param - 1) % 12],
						(instrument[currinstr].line[i].param - 1) / 12);
				} else {
					snprintf(buf, sizeof(buf), "---");
				}
			} else {
				snprintf(buf, sizeof(buf), "%02x", instrument[currinstr].line[i].param);
			}
			addstr(buf);
			attrset(A_NORMAL);
		}
	}
}

void drawmodeinfo(int x, int y) {
	switch(playmode) {
		case PM_IDLE:
			if(currtab == 2) {
				mvaddstr(y, x, "PLAY         IDLE space-> EDIT");
			} else {
				mvaddstr(y, x, "PLAY <-enter IDLE space-> EDIT");
			}
			attrset(A_REVERSE);
			mvaddstr(y, x + 13, "IDLE");
			attrset(A_NORMAL);
			break;
		case PM_PLAY:
			mvaddstr(y, x, "PLAY space-> IDLE         EDIT");
			attrset(A_REVERSE);
			mvaddstr(y, x + 0, "PLAY");
			attrset(A_NORMAL);
			break;
		case PM_EDIT:
			mvaddstr(y, x, "PLAY         IDLE <-space EDIT");
			attrset(A_REVERSE);
			mvaddstr(y, x + 26, "EDIT");
			attrset(A_NORMAL);
			break;
	}
}

void optimize() {
	u8 used[256], replace[256];
	int i, j;

	memset(used, 0, sizeof(used));
	for(i = 0; i < songlen; i++) {
		for(j = 0; j < 4; j++) {
			used[song[i].track[j]] = 1;
		}
	}

	j = 1;
	replace[0] = 0;
	for(i = 1; i < 256; i++) {
		if(used[i]) {
			replace[i] = j;
			j++;
		} else {
			replace[i] = 0;
		}
	}

	for(i = 1; i < 256; i++) {
		if(replace[i] && replace[i] != i) {
			memcpy(&track[replace[i]], &track[i], sizeof(struct track));
		}
	}

	for(i = 0; i < songlen; i++) {
		for(j = 0; j < 4; j++) {
			song[i].track[j] = replace[song[i].track[j]];
		}
	}

	for(i = 1; i < 256; i++) {
		u8 last = 255;

		for(j = 0; j < tracklen; j++) {
			if(track[i].line[j].instr) {
				if(track[i].line[j].instr == last) { track[i].line[j].instr = 0; } else { last = track[i].line[j].instr;
				}
			}
		}
	}
}

static FILE *exportfile = 0;
static int exportbits = 0;
static int exportcount = 0;
static int exportseek = 0;

void putbit(int x) {
	if(x) {
		exportbits |= (1 << exportcount);
	}
	exportcount++;
	if(exportcount == 8) {
		if(exportfile) {
			fprintf(exportfile, "\t.byte\t0x%02x\n", exportbits);
		}
		exportseek++;
		exportbits = 0;
		exportcount = 0;
	}
}

void exportchunk(int data, int bits) {
	int i;

	for(i = 0; i < bits; i++) {
		putbit(!!(data & (1 << i)));
	}
}

int alignbyte() {
	if(exportcount) {
		if(exportfile) {
			fprintf(exportfile, "\t.byte\t0x%02x\n", exportbits);
		}
		exportseek++;
		exportbits = 0;
		exportcount = 0;
	}
	if(exportfile) fprintf(exportfile, "\n");
	return exportseek;
}

int packcmd(u8 ch) {
	if(!ch) return 0;
	if(strchr(validcmds, ch)) {
		return strchr(validcmds, ch) - validcmds;
	}
	return 0;
}

void exportdata(FILE *f, int maxtrack, int *resources) {
	int i, j;
	int nres = 0;

	exportfile = f;
	exportbits = 0;
	exportcount = 0;
	exportseek = 0;

	for(i = 0; i < 16 + maxtrack; i++) {
		exportchunk(resources[i], 13);
	}

	resources[nres++] = alignbyte();

	for(i = 0; i < songlen; i++) {
		for(j = 0; j < 4; j++) {
			if(song[i].transp[j]) {
				exportchunk(1, 1);
				exportchunk(song[i].track[j], 6);
				exportchunk(song[i].transp[j], 4);
			} else {
				exportchunk(0, 1);
				exportchunk(song[i].track[j], 6);
			}
		}
	}

	for(i = 1; i < 16; i++) {
		resources[nres++] = alignbyte();

		if(instrument[i].length > 1) {
			for(j = 0; j < instrument[i].length; j++) {
				exportchunk(packcmd(instrument[i].line[j].cmd), 8);
				exportchunk(instrument[i].line[j].param, 8);
			}
		}

		exportchunk(0, 8);
	}

	for(i = 1; i <= maxtrack; i++) {
		resources[nres++] = alignbyte();

		for(j = 0; j < tracklen; j++) {
			u8 cmd = packcmd(track[i].line[j].cmd[0]);

			exportchunk(!!track[i].line[j].note, 1);
			exportchunk(!!track[i].line[j].instr, 1);
			exportchunk(!!cmd, 1);

			if(track[i].line[j].note) {
				exportchunk(track[i].line[j].note, 7);
			}

			if(track[i].line[j].instr) {
				exportchunk(track[i].line[j].instr, 4);
			}

			if(cmd) {
				exportchunk(cmd, 4);
				exportchunk(track[i].line[j].param[0], 8);
			}
		}
	}
}

void export() {
	FILE *f = fopen("exported.s", "w");
	FILE *hf = fopen("exported.h", "w");
	int i, j;
	int maxtrack = 0;
	int resources[256];

	exportfile = 0;
	exportbits = 0;
	exportcount = 0;
	exportseek = 0;

	for(i = 0; i < songlen; i++) {
		for(j = 0; j < 4; j++) {
			if(maxtrack < song[i].track[j]) maxtrack = song[i].track[j];
		}
	}

	fprintf(f, "\t.global\tsongdata\n\n");

	fprintf(hf, "#define MAXTRACK\t0x%02x\n", maxtrack);
	fprintf(hf, "#define SONGLEN\t\t0x%02x\n", songlen);

	fprintf(f, "songdata:\n");

	exportdata(0, maxtrack, resources);

	fprintf(f, "# ");
	for(i = 0; i < 16 + maxtrack; i++) {
		fprintf(f, "%04x ", resources[i]);
	}
	fprintf(f, "\n");

	exportdata(f, maxtrack, resources);

	fclose(f);
	fclose(hf);
}

//void vicmd() {
//}

void initjoystick() {
	SDL_Event event;
	SDL_Joystick *joystick = NULL;

	SDL_JoystickEventState(SDL_ENABLE);
	if (SDL_NumJoysticks > 0) {
		joystick = SDL_JoystickOpen(0);
	}

	sdlmainloop(event, joystick);
}

void sdlmainloop(SDL_Event event, SDL_Joystick *joystick) {
	SDL_JoystickEventState(SDL_ENABLE);
	joystick = SDL_JoystickOpen(0);

	// wtf does this do
	while(SDL_PollEvent(&event)) { //nik its just checking if a bit is set :) 
	//while(!sdl_finished) {       //   oh hmmm.. ok.
		switch(event.type) {  
			case SDL_KEYDOWN:
				break;

			case SDL_JOYBUTTONDOWN:
				switch(event.jbutton.button) {
				// should probably figure out what these are
				// gotta make them configurable, too
					case 0:
						currbutt = 0;
						display("0");
						break;
					case 1:
						currbutt = 1;
						display("1");
						break;
					case 2:
						currbutt = 2;
						display("2");
						break;
					case 3:
						currbutt = 3;
						display("3");
						break;
					default:
						display("unknown joystick button");
						break;
				}
				break;

			case SDL_QUIT:
				sdl_finished = true;
				break;

			default:
				break;
		}
	}
}

/* actions are anything that can be repeated by entering a number beforehand */
enum {
	ACT_MVLEFT,
	ACT_MVRIGHT,
	ACT_MVUP,
	ACT_MVDOWN,
	ACT_BIGMVUP,
	ACT_BIGMVDOWN,
	ACT_VIEWPHRASEINC,
	ACT_VIEWPHRASEDEC,
	ACT_VIEWINSTRINC,
	ACT_VIEWINSTRDEC,
	ACT_NOTEINC,
	ACT_NOTEDEC,
	ACT_INSTRINC,
	ACT_INSTRDEC,
	ACT_FXINC,
	ACT_FXDEC,
	ACT_PARAMINC,
	ACT_PARAMDEC,
	ACT_ADDLINE,
	ACT_DELLINE,
	ACT_CLRONETHING,
	ACT_CLRITALL
};
/* execute an action */
void actexec (int act) {
	int i;
	for (i=0;i<cmdrepeatnum;i++) {
		switch (act) {
			case ACT_MVLEFT:
				switch(currtab) {
					case 0:
						if(songx) songx--;
						break;
					case 1:
						if(trackx) trackx--;
						break;
					case 2:
						if(instrx) instrx--;
					break;
				}
				break;
			case ACT_MVRIGHT:
				switch(currtab) {
					case 0:
						if(songx < 15) songx++;
						break;
					case 1:
						if(trackx < 8) trackx++;
						break;
					case 2:
						if(instrx < 2) instrx++;
						break;
				}
				break;
			case ACT_MVUP:
				switch(currtab) {
					case 0:
						if(songy) songy--;
						break;
					case 1:
						if(tracky) {
							tracky--;
						} else {
							tracky = tracklen - 1;
						}
						break;
					case 2:
						if(instry) instry--;
						break;
				}
				break;
			case ACT_MVDOWN:
				switch(currtab) {
					case 0:
						if(songy < songlen - 1) songy++;
						break;
					case 1:
						if(tracky < tracklen - 1) {
							tracky++;
						} else {
							tracky = 0;
						}
						break;
					case 2:
						if(instry < instrument[currinstr].length - 1) instry++;
						break;
				}
				break;
			case ACT_BIGMVUP:
				switch(currtab) {
					case 0:
						if(songy >= 8) {
							songy -= 8;
						} else {
							songy = 0;
						}
						break;
					case 1:
						if(tracky >= 8) {
							tracky -= 8;
						} else {
							tracky = 0;
						}
						break;
					case 2:
						if(instry >= 8) instry -= 8;
						break;
				}
				break;
			case ACT_BIGMVDOWN:
				switch(currtab) {
					case 0:
						if(songy < songlen - 8) {
							songy += 8;
						} else {
							songy = songlen - 1;
						}
						break;
					case 1:
						if(tracky < tracklen - 8) {
							tracky += 8;
						} else {
							tracky = tracklen - 1;
						}
						break;
					case 2:
						if(instry < instrument[currinstr].length - 8) instry += 8;
						break;
				}
				break;
			case ACT_VIEWPHRASEINC:
				if(currtrack < 255) {
					currtrack++;
					if (playmode == PM_PLAY) {
						startplaytrack(currtrack);
					}
				}
				break;
			case ACT_VIEWPHRASEDEC:
				if(currtrack > 1) {
					currtrack--;
					if (playmode == PM_PLAY) {
						startplaytrack(currtrack);
					}
				}
				break;
			case ACT_VIEWINSTRINC:
				if(currinstr < 255) currinstr++;
				break;
			case ACT_VIEWINSTRDEC:
				if(currinstr > 1) currinstr--;
				break;
			case ACT_NOTEINC:
						// if current note < H7
				if ( track[currtrack].line[tracky].note < 96 ) {
					track[currtrack].line[tracky].note++;
				} else {
					track[currtrack].line[tracky].note = 0;
				}
				break;
			case ACT_NOTEDEC:
				if ( track[currtrack].line[tracky].note > 0 ) {
					track[currtrack].line[tracky].note--;
				} else {
					track[currtrack].line[tracky].note = 96;
				}
				break;
			case ACT_INSTRINC:
				switch (trackx) {
					case 1:
						SETHI(track[currtrack].line[tracky].instr,
								hexinc(track[currtrack].line[tracky].instr >> 4) );
						SETLO(track[currtrack].line[tracky].instr,
								hexdec(track[currtrack].line[tracky].instr & 0x0f) );
					case 2:
						SETLO(track[currtrack].line[tracky].instr,
								hexinc(track[currtrack].line[tracky].instr & 0x0f) );
				}
				break;
			case ACT_INSTRDEC:
				switch (trackx) {
					case 1:
						SETHI(track[currtrack].line[tracky].instr,
								hexdec(track[currtrack].line[tracky].instr >> 4) );
						SETLO(track[currtrack].line[tracky].instr,
								hexinc(track[currtrack].line[tracky].instr & 0x0f) );
					case 2:
						SETLO(track[currtrack].line[tracky].instr,
								hexdec(track[currtrack].line[tracky].instr & 0x0f) );
				}
				break;
				// TODO: FXINC and FXDEC
			case ACT_FXINC:
				//strlen(validcmds);
				break;
			case ACT_FXDEC:
				break;
			case ACT_PARAMINC:
				if (trackx==4 || trackx==7) {
					SETHI(track[currtrack].line[tracky].param[trackx%2],
							hexinc(track[currtrack].line[tracky].param[0] >> 4) );
				} else if (trackx==5 || trackx==8) {
					SETLO(track[currtrack].line[tracky].param[(trackx-1)%2],
							hexinc(track[currtrack].line[tracky].param[(trackx-1)%2] & 0x0f) );
				}
				break;
			case ACT_PARAMDEC:
				if (trackx==4 || trackx==7) {
					SETHI(track[currtrack].line[tracky].param[trackx%2],
							hexdec(track[currtrack].line[tracky].param[trackx%2] >> 4) );
				} else if (trackx==5 || trackx==8) {
					SETLO(track[currtrack].line[tracky].param[(trackx-1)%2],
							hexdec(track[currtrack].line[tracky].param[(trackx-1)%2] & 0x0f) );
				}
				break;
			case ACT_ADDLINE:
				if(currtab == 2) {
					struct instrument *in = &instrument[currinstr];

					if(in->length < 256) {
						memmove(&in->line[instry + 2], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
						instry++;
						in->length++;
						in->line[instry].cmd = '0';
						in->line[instry].param = 0;
					}
				} else if(currtab == 0) {
					if(songlen < 256) {
						memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
						songy++;
						songlen++;
						memset(&song[songy], 0, sizeof(struct songline));
					}
				}
				break;
			case ACT_DELLINE:
				if(currtab == 2) {
					struct instrument *in = &instrument[currinstr];

					if(in->length > 1) {
						memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
						in->length--;
						if(instry >= in->length) instry = in->length - 1;
					}
				} else if(currtab == 0) {
					if(songlen > 1) {
						memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
						songlen--;
						if(songy >= songlen) songy = songlen - 1;
					}
				}
				break;
			case ACT_CLRONETHING:
			// TODO: finish this
				if(currtab == 0) {
				} else if (currtab == 1) {
					switch (trackx) {
						case 0:
							track[currtrack].line[tracky].note = 0;
							track[currtrack].line[tracky].instr = 0;
							break;
						case 1:
							SETHI(track[currtrack].line[tracky].instr, 0);
							break;
						case 2:
							SETLO(track[currtrack].line[tracky].instr, 0);
							break;
						case 3:
							track[currtrack].line[tracky].cmd[0] = 0;
							break;
						case 4:
							SETHI(track[currtrack].line[tracky].param[0],0);
							break;
						case 5:
							SETLO(track[currtrack].line[tracky].param[0],0);
							break;
						case 6:
							track[currtrack].line[tracky].cmd[1] = 0;
							break;
						case 7:
							SETHI(track[currtrack].line[tracky].param[1],0);
							break;
						case 8:
							SETLO(track[currtrack].line[tracky].param[1],0);
							break;
						default:
							display("lol wut");
							break;
					}
				} else if (currtab == 2) {
				}
				break;
			case ACT_CLRITALL:
			// TODO: finish this
				if(currtab == 0) {
				} else if (currtab == 1) {
					track[currtrack].line[tracky].note = 0;
					track[currtrack].line[tracky].instr = 0;
					SETHI(track[currtrack].line[tracky].instr, 0);
					SETLO(track[currtrack].line[tracky].instr, 0);
					track[currtrack].line[tracky].cmd[0] = 0;
					SETHI(track[currtrack].line[tracky].param[0],0);
					SETLO(track[currtrack].line[tracky].param[0],0);
					track[currtrack].line[tracky].cmd[1] = 0;
					SETHI(track[currtrack].line[tracky].param[1],0);
					SETLO(track[currtrack].line[tracky].param[1],0);
				} else if (currtab == 2) {
				}
			break;
		} // end switch
	} // end for
	cmdrepeatnum = 1;
	cmdrepeat = false;
}

// waits for next char from getch and matches it with the parameter
// Look at this stupid function.. i wish i could make it more clear and have
// it still work.
bool nextchar(char ch) {
	char newch;
	newch = getch();
	while (newch == ERR) {
		if (getch() == ch || newch == ch) {
			return true;
		}
		newch = getch();
	}
	return (newch == ch);
}

/* vi insert mode */
void insertroutine() {
	int c, x;
	playmode = PM_EDIT;
	insertmode = true;
	drawgui();
	for(;;) {
		if ((c = getch()) != ERR) switch(c) {
			case KEY_ESCAPE:
				playmode = PM_IDLE;
				insertmode = false;
				guiloop();
			case 'h':
			case KEY_LEFT:
				actexec(ACT_MVLEFT);
				break;
			case 'j':
			case KEY_DOWN:
				actexec(ACT_MVDOWN);
				break;
			case 'k':
			case KEY_UP:
				actexec(ACT_MVUP);
				break;
			case 'l':
			case KEY_RIGHT:
				actexec(ACT_MVRIGHT);
				break;
			case CTRL('J'):
				if (currtab == 2) {
					actexec(ACT_VIEWINSTRDEC);
				} else if (currtab == 1) {
					actexec(ACT_VIEWPHRASEDEC);
				}
				break;
			case CTRL('K'):
				if (currtab == 2) {
					actexec(ACT_VIEWINSTRINC);
				} else if (currtab == 1) {
					actexec(ACT_VIEWPHRASEINC);
				}
				break;
			case CTRL('H'):
				currtab--;
				if(currtab < 0)
					currtab = 2;
				break;
			case CTRL('L'):
				currtab++;
				currtab %= 3;
				break;
			case 'Z':
				if (nextchar('Z')) {
					savefile(filename);
					erase();
					refresh();
					endwin();
					exit(0);
				}
				break;
			case ' ':
				silence();
				playmode = PM_IDLE;
				insertmode = false;
				guiloop();
				break;
			case 13:  // Enter key
				if(currtab != 2) {
					playmode = PM_PLAY;
					if(currtab == 1) {
						silence();
						startplaytrack(currtrack);
					} else if(currtab == 0) {
						silence();
						startplaysong(songy);
					}
				}
				break;
			case '`':
				if(currtab == 0) {
					int t = song[songy].track[songx / 4];
					if(t) currtrack = t;
					currtab = 1;
				} else if(currtab == 1) {
					currtab = 0;
				}
				break;
			default:
				x = hexdigit(c);
				if(x >= 0) {
					if(currtab == 2
					&& instrx > 0
					&& instrument[currinstr].line[instry].cmd != '+'
					&& instrument[currinstr].line[instry].cmd != '=') {
						switch(instrx) {
							case 1: SETHI(instrument[currinstr].line[instry].param, x); break;
							case 2: SETLO(instrument[currinstr].line[instry].param, x); break;
						}
					}
					if(currtab == 1 && trackx > 0) {
						switch(trackx) {
							case 1: SETHI(track[currtrack].line[tracky].instr, x); break;
							case 2: SETLO(track[currtrack].line[tracky].instr, x); break;
							case 4: if(track[currtrack].line[tracky].cmd[0])
								SETHI(track[currtrack].line[tracky].param[0], x); break;
							case 5: if(track[currtrack].line[tracky].cmd[0])
								SETLO(track[currtrack].line[tracky].param[0], x); break;
							case 7: if(track[currtrack].line[tracky].cmd[1])
								SETHI(track[currtrack].line[tracky].param[1], x); break;
							case 8: if(track[currtrack].line[tracky].cmd[1])
								SETLO(track[currtrack].line[tracky].param[1], x); break;
						}
					}
					if(currtab == 0) {
						switch(songx & 3) {
							case 0: SETHI(song[songy].track[songx / 4], x); break;
							case 1: SETLO(song[songy].track[songx / 4], x); break;
							case 2: SETHI(song[songy].transp[songx / 4], x); break;
							case 3: SETLO(song[songy].transp[songx / 4], x); break;
						}
					}
				}
				x = freqkey(c);
				if(x >= 0) {
					if(currtab == 2
					&& instrx
					&& (instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '=')) {
						instrument[currinstr].line[instry].param = x;
					}
					if(currtab == 1 && !trackx) {
						track[currtrack].line[tracky].note = x;
						if(x) {
							track[currtrack].line[tracky].instr = currinstr;
						} else {
							track[currtrack].line[tracky].instr = 0;
						}
						tracky++;
						tracky %= tracklen;
						if(x) iedplonk(x, currinstr);
					}
				}
				if(currtab == 2 && instrx == 0) {
					if(strchr(validcmds, c)) {
						instrument[currinstr].line[instry].cmd = c;
					}
				}
				if(currtab == 1 && (trackx == 3 || trackx == 6 || trackx == 9)) {
					if(strchr(validcmds, c)) {
						if(c == '.' || c == '0') c = 0;
						track[currtrack].line[tracky].cmd[(trackx - 3) / 3] = c;
					}
				}
				if(c == 'I') {
					if(currtab == 2) {
						struct instrument *in = &instrument[currinstr];

						if(in->length < 256) {
							memmove(&in->line[instry + 1], &in->line[instry + 0], sizeof(struct instrline) * (in->length - instry));
							in->length++;
							in->line[instry].cmd = '0';
							in->line[instry].param = 0;
						}
					} else if(currtab == 0) {
						if(songlen < 256) {
							memmove(&song[songy + 1], &song[songy + 0], sizeof(struct songline) * (songlen - songy));
							songlen++;
							memset(&song[songy], 0, sizeof(struct songline));
						}
					}
				} else if(c == 'D') {
					if(currtab == 2) {
						struct instrument *in = &instrument[currinstr];

						if(in->length > 1) {
							memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
							in->length--;
							if(instry >= in->length) instry = in->length - 1;
						}
					} else if(currtab == 0) {
						if(songlen > 1) {
							memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
							songlen--;
							if(songy >= songlen) songy = songlen - 1;
						}
					}
				}
			} else if(playmode == PM_IDLE) {
				x = freqkey(c);

				if(x > 0) {
					iedplonk(x, currinstr);
				}
			break;
		}
		drawgui();
	}
}

/* vi command mode */
void commandroutine() {
	int c;
	cmdstr = ":";
	cmdmode = true;
	drawgui();
	for(;;) {
		if ((c = getch()) != ERR) switch(c) {
			case KEY_ESCAPE:
				cmdstr = "";
				cmdmode = false;
				break;
			default:
				winheight = getmaxy(stdscr);
				//cmdstr = strcat(cmdstr,c);
				//mvaddstr(winheight-1, strlen(cmdstr)+1, c);
				break;
		}
	drawgui();
	}
}

int char2int(char ch) {
	if (isdigit(ch)) {
		return (int)ch - '0';
	}
	return -1;
}

/* vi mode and non-vi mode */
void handleinput() {
	int c, x;

	if (vimode) {
		if ((c = getch()) != ERR) {

			/* Repeat? */
			if (isdigit(c)) {
				if (!cmdrepeat) {
					cmdrepeatnum = char2int(c);
				} else {
					cmdrepeatnum = (cmdrepeatnum*10) + char2int(c);
				}
			}

			switch(c) {
			/* add line */
			case 'a':
				if(currtab == 2) {
					struct instrument *in = &instrument[currinstr];

					if(in->length < 256) {
						memmove(&in->line[instry + 2], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
						instry++;
						in->length++;
						in->line[instry].cmd = '0';
						in->line[instry].param = 0;
					}
				} else if(currtab == 0) {
					if(songlen < 256) {
						memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
						songy++;
						songlen++;
						memset(&song[songy], 0, sizeof(struct songline));
					}
				}
				break;
			/* delete line */
			// TODO: 'd' then direction
			case 'd':
				if (nextchar('d')) {
					if(currtab == 2) {
						struct instrument *in = &instrument[currinstr];

						if(in->length > 1) {
							memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
							in->length--;
							if(instry >= in->length) instry = in->length - 1;
						}
					} else if(currtab == 0) {
						if(songlen > 1) {
							memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
							songlen--;
							if(songy >= songlen) songy = songlen - 1;
						}
					}
				}
				break;
			/* Clear */
			case 'x':
				actexec(ACT_CLRONETHING);
				break;
			case 'X':
				actexec(ACT_CLRITALL);
				break;
			case 13:  // Enter key
				if(currtab != 2) {
					playmode = PM_PLAY;
					if(currtab == 1) {
						silence();
						startplaytrack(currtrack);
					} else if(currtab == 0) {
						silence();
						startplaysong(songy);
					}
				}
				break;
			case 'Z':
				if (nextchar('Z')) {
					savefile(filename);
					erase();
					refresh();
					endwin();
					exit(0);
				}
				break;
			/* Enter command mode */
			case ':':
				commandroutine();
				break;
			case ' ':
				silence();
				if(playmode == PM_IDLE) {
					playmode = PM_EDIT;
				} else {
					playmode = PM_IDLE;
				}
				break;
			case '`':
				if(currtab == 0) {
					int t = song[songy].track[songx / 4];
					if(t) currtrack = t;
					currtab = 1;
				} else if(currtab == 1) {
					currtab = 0;
				}
				break;
			/* Enter insert mode */
			case 'i':
				insertroutine();
				break;
			/* Add new line and enter insert mode */
			case 'o':
				if(currtab == 2) {
					struct instrument *in = &instrument[currinstr];

					if(in->length < 256) {
						memmove(&in->line[instry + 2], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
						instry++;
						in->length++;
						in->line[instry].cmd = '0';
						in->line[instry].param = 0;
					}
				} else if(currtab == 0) {
					if(songlen < 256) {
						memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
						songy++;
						songlen++;
						memset(&song[songy], 0, sizeof(struct songline));
					}
				}
				insertroutine();
				break;
			case 'h':
			case KEY_LEFT:
				actexec(ACT_MVLEFT);
				break;
			case 'j':
			case KEY_DOWN:
				actexec(ACT_MVDOWN);
				break;
			case 'k':
			case KEY_UP:
				actexec(ACT_MVUP);
				break;
			case 'l':
			case KEY_RIGHT:
				actexec(ACT_MVRIGHT);
				break;
			case '<':
				if(octave) octave--;
				break;
			case '>':
				if(octave < 8) octave++;
				break;
			// TODO: 'J' and 'K' should all call the same actexec function
			// and the code to handle where the cursor is will be in there.
			case 'J':
				if(currtab == 1) {
					switch (trackx) {
						case 0:
							actexec(ACT_NOTEDEC);
							break;
						case 1:
							actexec(ACT_INSTRDEC);
							break;
						case 2:
							actexec(ACT_INSTRDEC);
							break;
						case 3:
							actexec(ACT_FXDEC);	
							break;
						case 4:
							actexec(ACT_PARAMDEC);	
							break;
						case 6:
							actexec(ACT_FXDEC);	
							break;
						case 7:
							actexec(ACT_PARAMDEC);	
							break;
						case 9:
							actexec(ACT_FXDEC);	
							break;
						default:
							display("lol wut");
							break;
						}
					}
				break;
			case 'K':
				if(currtab == 1) {
					switch (trackx) {
						case 0:
							actexec(ACT_NOTEINC);
							break;
						case 1:
							actexec(ACT_INSTRINC);
							break;
						case 2:
							actexec(ACT_INSTRINC);
							break;
						case 3:
							actexec(ACT_FXINC);	
							break;
						case 4:
							actexec(ACT_PARAMINC);	
							break;
						case 6:
							actexec(ACT_FXINC);	
							break;
						case 7:
							actexec(ACT_PARAMINC);	
							break;
						case 9:
							actexec(ACT_FXINC);	
							break;
						default:
							display("lol wut");
							break;
					}
				}
				break;
			case CTRL('J'):
				if (currtab == 2) {
					actexec(ACT_VIEWINSTRDEC);
				} else if (currtab == 1) {
					actexec(ACT_VIEWPHRASEDEC);
				}
				break;
			case CTRL('K'):
				if (currtab == 2) {
					actexec(ACT_VIEWINSTRINC);
				} else if (currtab == 1) {
					actexec(ACT_VIEWPHRASEINC);
				}
				break;
			case CTRL('H'):
				currtab--;
				if(currtab < 0)
					currtab = 2;
				break;
			case CTRL('L'):
				currtab++;
				currtab %= 3;
				break;
			case KEY_TAB:
				currtab++;
				currtab %= 3;
				break;
			case CTRL('B'):
				actexec(ACT_BIGMVUP);
				break;
			case CTRL('F'):
				actexec(ACT_BIGMVDOWN);
				break;
			case CTRL('P'):
				vimode = false;
				break;
			default:
				break;
			}
		}
	/* non-vi mode */
	} else {
		if((c = getch()) != ERR) switch(c) {
			case 10:
			case 13:
				if(currtab != 2) {
					playmode = PM_PLAY;
					if(currtab == 1) {
						startplaytrack(currtrack);
					} else if(currtab == 0) {
						startplaysong(songy);
					}
				}
				break;
			case CTRL('P'):
				playmode = PM_IDLE;
				vimode = true;
				break;
			case ' ':
				silence();
				if(playmode == PM_IDLE) {
					playmode = PM_EDIT;
				} else {
					playmode = PM_IDLE;
				}
				break;
			case KEY_TAB:
				currtab++;
				currtab %= 3;
				break;
			case CTRL('E'):
				erase();
				refresh();
				endwin();
				exit(0);
				break;
			case CTRL('W'):
				savefile(filename);
				display("*saved*");
				break;
			case '<':
				if(octave) octave--;
				break;
			case '>':
				if(octave < 8) octave++;
				break;
			case '[':
				if(currinstr > 1) currinstr--;
				break;
			case ']':
				if(currinstr < 255) currinstr++;
				break;
			case '{':
				if(currtrack > 1) currtrack--;
				break;
			case '}':
				if(currtrack < 255) currtrack++;
				break;
			case '`':
				if(currtab == 0) {
					int t = song[songy].track[songx / 4];
					if(t) currtrack = t;
					currtab = 1;
				} else if(currtab == 1) {
					currtab = 0;
				}
				break;

			case '#':
				optimize();
				break;
			case '%':
				export();
				break;
			case KEY_LEFT:
				actexec(ACT_MVLEFT);
				break;
			case KEY_DOWN:
				actexec(ACT_MVDOWN);
				break;
			case KEY_UP:
				actexec(ACT_MVUP);
				break;
			case KEY_RIGHT:
				actexec(ACT_MVRIGHT);
				break;
			case 'C':
				if(currtab == 2) {
					memcpy(&iclip, &instrument[currinstr], sizeof(struct instrument));
				} else if(currtab == 1) {
					memcpy(&tclip, &track[currtrack], sizeof(struct track));
				}
				break;
			case 'V':
				if(currtab == 2) {
					memcpy(&instrument[currinstr], &iclip, sizeof(struct instrument));
				} else if(currtab == 1) {
					memcpy(&track[currtrack], &tclip, sizeof(struct track));
				}
				break;
			default:
				if(playmode == PM_EDIT) {
					x = hexdigit(c);
					if(x >= 0) {
						if(currtab == 2
						&& instrx > 0
						&& instrument[currinstr].line[instry].cmd != '+'
						&& instrument[currinstr].line[instry].cmd != '=') {
							switch(instrx) {
								case 1: SETHI(instrument[currinstr].line[instry].param, x); break;
								case 2: SETLO(instrument[currinstr].line[instry].param, x); break;
							}
						}
						if(currtab == 1 && trackx > 0) {
							switch(trackx) {
								case 1: SETHI(track[currtrack].line[tracky].instr, x); break;
								case 2: SETLO(track[currtrack].line[tracky].instr, x); break;
								case 4: if(track[currtrack].line[tracky].cmd[0])
									SETHI(track[currtrack].line[tracky].param[0], x); break;
								case 5: if(track[currtrack].line[tracky].cmd[0])
									SETLO(track[currtrack].line[tracky].param[0], x); break;
								case 7: if(track[currtrack].line[tracky].cmd[1])
									SETHI(track[currtrack].line[tracky].param[1], x); break;
								case 8: if(track[currtrack].line[tracky].cmd[1])
									SETLO(track[currtrack].line[tracky].param[1], x); break;
							}
						}
						if(currtab == 0) {
							switch(songx & 3) {
								case 0: SETHI(song[songy].track[songx / 4], x); break;
								case 1: SETLO(song[songy].track[songx / 4], x); break;
								case 2: SETHI(song[songy].transp[songx / 4], x); break;
								case 3: SETLO(song[songy].transp[songx / 4], x); break;
							}
						}
					}
					x = freqkey(c);
					if(x >= 0) {
						if(currtab == 2
						&& instrx
						&& (instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '=')) {
							instrument[currinstr].line[instry].param = x;
						}
						if(currtab == 1 && !trackx) {
							track[currtrack].line[tracky].note = x;
							if(x) {
								track[currtrack].line[tracky].instr = currinstr;
							} else {
								track[currtrack].line[tracky].instr = 0;
							}
							tracky++;
							tracky %= tracklen;
							if(x) iedplonk(x, currinstr);
						}
					}
					if(currtab == 2 && instrx == 0) {
						if(strchr(validcmds, c)) {
							instrument[currinstr].line[instry].cmd = c;
						}
					}
					if(currtab == 1 && (trackx == 3 || trackx == 6 || trackx == 9)) {
						if(strchr(validcmds, c)) {
							if(c == '.' || c == '0') c = 0;
							track[currtrack].line[tracky].cmd[(trackx - 3) / 3] = c;
						}
					}
					if(c == 'A') {
						if(currtab == 2) {
							struct instrument *in = &instrument[currinstr];

							if(in->length < 256) {
								memmove(&in->line[instry + 2], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
								instry++;
								in->length++;
								in->line[instry].cmd = '0';
								in->line[instry].param = 0;
							}
						} else if(currtab == 0) {
							if(songlen < 256) {
								memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
								songy++;
								songlen++;
								memset(&song[songy], 0, sizeof(struct songline));
							}
						}
					} else if(c == 'I') {
						if(currtab == 2) {
							struct instrument *in = &instrument[currinstr];

							if(in->length < 256) {
								memmove(&in->line[instry + 1], &in->line[instry + 0], sizeof(struct instrline) * (in->length - instry));
								in->length++;
								in->line[instry].cmd = '0';
								in->line[instry].param = 0;
							}
						} else if(currtab == 0) {
							if(songlen < 256) {
								memmove(&song[songy + 1], &song[songy + 0], sizeof(struct songline) * (songlen - songy));
								songlen++;
								memset(&song[songy], 0, sizeof(struct songline));
							}
						}
					} else if(c == 'D') {
						if(currtab == 2) {
							struct instrument *in = &instrument[currinstr];

							if(in->length > 1) {
								memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
								in->length--;
								if(instry >= in->length) instry = in->length - 1;
							}
						} else if(currtab == 0) {
							if(songlen > 1) {
								memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
								songlen--;
								if(songy >= songlen) songy = songlen - 1;
							}
						}
					}
				} else if(playmode == PM_IDLE) {
					x = freqkey(c);

					if(x > 0) {
						iedplonk(x, currinstr);
					}
				}
				break;
		}
	}
}

void display(char *str) {
	disptick = 1000;
	dispmesg = str;
}

void drawgui() {
	char buf[1024];
	int lines = LINES, cols = 79;
	int songcols[] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22};
	int trackcols[] = {0, 4, 5, 7, 8, 9, 11, 12, 13};
	int instrcols[] = {0, 2, 3};

	erase();
	mvaddstr(0, 0, "music chip tracker 0.1 by lft");
	mvaddstr(1, 0, "press ^P to switch keybindings");
	snprintf(buf, sizeof(buf), "Octave:   %d <>", octave);
	mvaddstr(2, cols - 14, buf);
	mvaddstr(3, cols - 14, "^W)rite ^E)xit");

	//snprintf(buf, sizeof(buf), "^F)ilename:        %s", filename);
	//mvaddstr(2, 15, buf);

	mvaddstr(5, 0, "Song");
	drawsonged(0, 6, lines - 12);

	snprintf(buf, sizeof(buf), "Track %02x {}", currtrack);
	mvaddstr(5, 29, buf);
	drawtracked(29, 6, lines - 8);

	snprintf(buf, sizeof(buf), "Instr. %02x []", currinstr);
	mvaddstr(5, 49, buf);
	drawinstred(49, 6, lines - 12);

	if (currbutt > -1) {
		snprintf(buf, sizeof(buf), "joybutton: %d", currbutt);
		mvaddstr(2, 0, buf);
	}

	// display
	if (disptick > 0) {
		mvaddstr(3, 0, dispmesg);
		disptick--;
	}

	if (vimode) {
		mvaddstr(0, cols - 30, "*(escape)vimode*");
	} else {
		drawmodeinfo(cols - 30, 0);
	}

	if (insertmode) {
		winheight = getmaxy(stdscr);
		mvaddstr(winheight-1, 0, "-- INSERT --");
	}

	if (cmdmode) {
		winheight = getmaxy(stdscr);
		mvaddstr(winheight-1, 0, cmdstr);
	}
    
	switch(currtab) {
		case 0:
			move(6 + songy - songoffs, 0 + 4 + songcols[songx]);
			break;
		case 1:
			move(6 + tracky - trackoffs, 29 + 4 + trackcols[trackx]);
			break;
		case 2:
			move(6 + instry - instroffs, 49 + 4 + instrcols[instrx]);
			break;
	}

	refresh();

	if (disptick > 0) {
		disptick--;
	}
}

void guiloop() {
	// don't treat the escape key like a meta key
	ESCDELAY = 50;
	for(;;) {
		drawgui();
		handleinput();
	}
}
