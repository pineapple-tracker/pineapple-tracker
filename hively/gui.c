#include <ncurses.h>
#include "hvl_replay.h"
#include "pineapple.h"
#include "gui.h"

int currtrack = 1;
int currinstr = 1;
int currtab = 0;
int songy = 0;

int disptick = 0;
char *dispmesg = "";
int note;

void _display(void);

void _display(void){
	int cx = (getmaxx(stdscr)/2)-(strlen(dispmesg)/2)-1;
	int cy = getmaxy(stdscr)/2;

	mvaddch(cy-1, cx, ACS_ULCORNER);
	for(int i=cx+1; i<cx+strlen(dispmesg)+1; i++)
		mvaddch(cy-1, i, ACS_HLINE);
	mvaddch(cy-1, cx+strlen(dispmesg)+1, ACS_URCORNER);

	mvaddch(cy, cx, ACS_VLINE);
	mvaddstr(cy, cx+1, dispmesg);
	mvaddch(cy, cx+strlen(dispmesg)+1, ACS_VLINE);

	mvaddch(cy+1, cx, ACS_LLCORNER);
	for(int i=cx+1; i<cx+strlen(dispmesg)+1; i++)
		mvaddch(cy+1, i, ACS_HLINE);
	mvaddch(cy+1, cx+strlen(dispmesg)+1, ACS_LRCORNER);
}

static char *notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-"};

char *keymap[2] = {
	"zsxdcvgbhnjm,l.;/",
	"q2w3er5t6y7ui9o0p"
};

void setdisplay(char *str){
	disptick = 350;
	dispmesg = str;
}

int freqkey(int c){
	char *s;
	int f = -1;

	if(c == '-' || c == KEY_DC) return 0;
	if(c > 0 && c < 256){
		s = strchr(keymap[0], c);
		if(s){
			f = (s - (keymap[0])) + octave * 12 + 1;
		}else{
			s = strchr(keymap[1], c);
			if(s){
				f = (s - (keymap[1])) + octave * 12 + 12 + 1;
			}
		}
	}
	if(f > 12 * 9 + 1) return -1;
	return f;
}

void iedplonk(int x) {
	setdisplay("\\o/");
	note = x;
	tune->curNote = x;
	//hvl_process_step(tune, &tune->ht_Voices[0]);
	//hvl_process_frame(tune, &tune->ht_Voices[0]);
	//hvl_set_audio(&tune->ht_Voices[0], tune->ht_Frequency);
	plonked = 1;
}

void initgui(){
	initscr();

	nonl();
	noecho();
	keypad(stdscr, FALSE);

	nodelay(stdscr, TRUE);
	
	//atexit(endwin());
}

void drawposed(){
	int i, j;
	char buf[1024];
	unsigned char trans;
	move(0,0);
	for(i = 0; i<tune->ht_PositionNr; i++){
		snprintf(buf, sizeof(buf), "%02x", i);
		addstr(buf);
		addch(ACS_VLINE);
		for(j= 0; j<4; j++){
			trans = tune->ht_Positions[i].pos_Transpose[j]; //this makes the transpose column display 'fe' instead of 'fffffffffe'..., which is weird since trans is unsigned
			snprintf(buf, sizeof(buf), "%02x:%02x", tune->ht_Positions[i].pos_Track[j], trans);
			addstr(buf);
			if(j !=3)
				addch(' ');
		}
		move(i + 1, 0);
		if(songy == i) attrset(A_BOLD);
	}
	int c = 0;
	snprintf(buf, sizeof(buf), "Name: %s", tune->ht_Name);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "PosNr: %02x", tune->ht_PosNr);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "PositionNr: %02x", tune->ht_PositionNr);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "Restart: %02x", tune->ht_Restart);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "NoteNr: %02x", tune->ht_NoteNr);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "TrackLength: %02x", tune->ht_TrackLength);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "TrackNr: %02x", tune->ht_TrackNr);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "Tempo: %02x", tune->ht_Tempo);
	mvaddstr(c++,60, buf);
	snprintf(buf, sizeof(buf), "StepWaitFrames: %02x", tune->ht_StepWaitFrames);
	mvaddstr(c++,60, buf);
	snprintf(buf, sizeof(buf), "SongEndReached: %02x", tune->ht_SongEndReached);
	mvaddstr(c++,60, buf);
	snprintf(buf, sizeof(buf), "Freq: %d", tune->ht_Frequency);
	mvaddstr(c++,60, buf);
	snprintf(buf, sizeof(buf), "GetNewPosition: %02x", tune->ht_GetNewPosition);
	mvaddstr(c++,60, buf);
	//for(int i = 0; i <tune->ht_Channels; i++) {
	for(int i = 0; i < 2; i++) {
		snprintf(buf, sizeof(buf), "VC %x TrackPeriod: %x", i, tune->ht_Voices[i].vc_TrackPeriod);
		mvaddstr(c++,60, buf);
		snprintf(buf, sizeof(buf), "VC %x SamplePos: %x", i, tune->ht_Voices[i].vc_SamplePos);
		mvaddstr(c++,60, buf);
		snprintf(buf, sizeof(buf), "VC %x Delta: %x", i, tune->ht_Voices[i].vc_Delta);
		mvaddstr(c++,60, buf);
		snprintf(buf, sizeof(buf), "VC %x WaveLength: %x", i, tune->ht_Voices[i].vc_WaveLength);
		mvaddstr(c++,60, buf);
		snprintf(buf, sizeof(buf), "VC %x PerfCurrent: %x", i, tune->ht_Voices[i].vc_PerfCurrent);
		mvaddstr(c++,60, buf);
		snprintf(buf, sizeof(buf), "VC %x PerfSpeed: %x", i, tune->ht_Voices[i].vc_PerfSpeed);
		mvaddstr(c++,60, buf);
		snprintf(buf, sizeof(buf), "VC %x FilterOn: %x", i, tune->ht_Voices[i].vc_FilterOn);
		mvaddstr(c++,60, buf);
		snprintf(buf, sizeof(buf), "VC %x FilterPos: %x", i, tune->ht_Voices[i].vc_FilterPos);
		mvaddstr(c++,60, buf);
		snprintf(buf, sizeof(buf), "VC %x SquarePos: %x", i, tune->ht_Voices[i].vc_SquarePos);
		mvaddstr(c++,60, buf);
	}
	snprintf(buf, sizeof(buf), "SpeedMultiplier: %02x", tune->ht_SpeedMultiplier);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "InstrumentNr: %02x", tune->ht_InstrumentNr);
	mvaddstr(c++, 60, buf);
	snprintf(buf, sizeof(buf), "Note: %s%d", notenames[(note-1) % 12], (note-1) / 12);
	mvaddstr(c++, 60, buf);
}

void drawtracked(){
	int i;
	char buf[1024];
	snprintf(buf, sizeof(buf), "Track: %02x", currtrack);
	mvaddstr(0, 26, buf);
	move(0, 35);
	for(i = 0; i < tune->ht_TrackLength; i++){
		snprintf(buf, sizeof(buf), "%02x", i);
		addstr(buf);
		addch(ACS_VLINE);

		if(tune->ht_Tracks[currtrack][i].stp_Note)
			//snprintf(buf, sizeof(buf), "%s%d  ", notenames[(tune->ht_Tracks[currtrack][i].stp_Note - 1) % 12], (tune->ht_Tracks[currtrack][i].stp_Note - 1) / 12);
			snprintf(buf, sizeof(buf), "%d ", tune->ht_Tracks[currtrack][i].stp_Note);
		else
			snprintf(buf, sizeof(buf), "--- ");
		addstr(buf);

		if(tune->ht_Tracks[currtrack][i].stp_Instrument)
			snprintf(buf, sizeof(buf), "%02x  ", tune->ht_Tracks[currtrack][i].stp_Instrument);
		else
			snprintf(buf, sizeof(buf), "--  ");
		addstr(buf);

		if(tune->ht_Tracks[currtrack][i].stp_FX)
			snprintf(buf, sizeof(buf), "%02x ", tune->ht_Tracks[currtrack][i].stp_FX);
		else
			snprintf(buf, sizeof(buf), "-- ");
		addstr(buf);

		if(tune->ht_Tracks[currtrack][i].stp_FXParam)
			snprintf(buf, sizeof(buf), "%02x ", tune->ht_Tracks[currtrack][i].stp_FXParam);
		else
			snprintf(buf, sizeof(buf), "-- ");
		addstr(buf);
		
		if(tune->ht_Tracks[currtrack][i].stp_FXb)
			snprintf(buf, sizeof(buf), "%02x ", tune->ht_Tracks[currtrack][i].stp_FX);
		else
			snprintf(buf, sizeof(buf), "-- ");
		addstr(buf);

		if(tune->ht_Tracks[currtrack][i].stp_FXbParam)
			snprintf(buf, sizeof(buf), "%02x ", tune->ht_Tracks[currtrack][i].stp_FXParam);
		else
			snprintf(buf, sizeof(buf), "-- ");
		addstr(buf);

		if((i == tune->ht_NoteNr))
			addch('*');

		move(i + 1, 35);
	}
}

void drawgui(){
	erase();
	drawposed();
	drawtracked();

	refresh();

	if(disptick > 0){
		_display();
		disptick--;
	}
}

void handleinput(){
	int c, x;
	if((c = getch()) != ERR){
		switch(c){
		case 'Q':
			erase();
			refresh();
			endwin();
			exit(0);
			break;
		case 'J':
			if(currtrack > 1)
				currtrack--;
			break;
		case 'K':
			if(currtrack < 0xff)
				currtrack++;
			break;
		case 'h':
		case KEY_LEFT:
			act_mvleft();
			break;
		case 'j':
		case KEY_DOWN:
			act_mvdown();
			break;
		case 'k':
		case KEY_UP:
			act_mvup();
			break;
		case 'l':
		case KEY_RIGHT:
			act_mvright();
			break;
		case ENTER:
			play = 1;
			break;
		case ' ':
			play = 0;
			plonked = 0;
			break;
		default:
			x = freqkey(c);	
			if(x > 0) iedplonk(x);
			break;
		}
	}
}

void guiloop(){
	ESCDELAY=50;
	for(;;){
		drawgui();
		handleinput();
	}

}
