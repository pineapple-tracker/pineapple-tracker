/* vi:set ts=8 sts=8 sw=8 noexpandtab: */

/* welcome to gui.c, enjoy your stay 8-) */

#include "pineapple.h"
#include "gui.h"

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  local vars                                                              .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
char *dispmesg = "";

static char *notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#",
		"G-", "G#", "A-", "A#", "H-"};

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  local functions                                                         .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
int _char2int(char ch);
void _display(void);

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  end local declarations                                                  .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//

char cmdstr[500] = "";

int disptick = 0;
int currmode = PM_NORMAL;
int octave = 4;
int songlen = 1;
int tracklen = TRACKLEN;
int currtrack = 1;
int currinstr = 1;
int currtab = 0;
int saved = 1;

int step = 1;

int cmdrepeat = 0;
int cmdrepeatnum = 1;
int lastrepeat = 1;

// 0 is like a blank command
char *validcmds = "0dfi@smtvw~+=*";

/*char *keymap[2] = {
	";oqejkixdbhmwnvsz",
	"'2,3.p5y6f7gc9r0l/="
};*/

char *keymap[2] = {
	"zsxdcvgbhnjm,l.;/",
	"q2w3er5t6y7ui9o0p"
};

/* hexinc and hexdec wrap around */
int hexinc(int x){
	return (x >= 0 && x <= 14)? x+1 : 0;
}
int hexdec(int x){
	return (x >= 1 && x <= 15)? x-1 : 15;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < int _char2int(char) >                                                 .|
///  Draws the instrument editor.                                            .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
int _char2int(char ch){
	if(isdigit(ch)){
		return (int)ch - '0';
	}
	return -1;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < int freqkey(int) >                                                    .|
///   Calculates the frequency of key c.                                     .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
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

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void initsonglines() >                                                .|
///   Calculates the frequency of key c.                                     .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void initsonglines(void){
	for(int i=0; i < songlen; i++){
		memmove(&song[i + 0], &song[i + 1], sizeof(struct songline) * (songlen - i - 1));
		if(i < 4){
			song[0].track[i] = 0x00;
			song[0].transp[i] = 0x00;
		}
	}
	songlen = 1;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void inittracks() >                                                   .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void inittracks(void){
	for(int i=0; i < 256; i++){
		for(int j=0; j < TRACKLEN; j++){
			track[i].line[j].note = 0x00;
			track[i].line[j].instr = 0x00;
			for(int k=0; k < 2; k++){
				track[i].line[j].cmd[k] = 0x0000;
				track[i].line[j].param[k] = 0x0000;
			}
		}
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void initinstrs() >                                                   .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void initinstrs(void){
	for(int i=1; i < 256; i++){
		instrument[i].length = 1;
		instrument[i].line[0].cmd = '0';
		instrument[i].line[0].param = 0;
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void readsong(int,int,u8) >                                           .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void readsong(int pos, int ch, u8 *dest){
	dest[0] = song[pos].track[ch];
	dest[1] = song[pos].transp[ch];
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void readtrack(int,int,trackline) >                                   .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void readtrack(int num, int pos, struct trackline *tl){
	tl->note = track[num].line[pos].note;
	tl->instr = track[num].line[pos].instr;
	tl->cmd[0] = track[num].line[pos].cmd[0];
	tl->cmd[1] = track[num].line[pos].cmd[1];
	tl->param[0] = track[num].line[pos].param[0];
	tl->param[1] = track[num].line[pos].param[1];
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void readinstr(int,int,u8) >                                          .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void readinstr(int num, int pos, u8 *il){
	if(pos >= instrument[num].length){
		il[0] = 0;
		il[1] = 0;
	}else{
		il[0] = instrument[num].line[pos].cmd;
		il[1] = instrument[num].line[pos].param;
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void exitgui() >                                                      .|
///  Exits the gui.                                                          .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void exitgui(){
	endwin();
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void initgui() >                                                      .|
///  Initializes the gui.                                                    .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void initgui(){
	initscr();

	//if(setlocale(LC_CTYPE,"en_US.utf8") != NULL) setdisplay("UTF-8 enabled!");

	// don't send newline on Enter key, and don't echo chars to the screen.
	nonl();
	noecho();

	// make sure behaviour for special keys like ^H isn't overridden
	keypad(stdscr, FALSE);

	nodelay(stdscr, TRUE);

	initinstrs();

	atexit(exitgui);
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void handleinput() >                                                  .|
///  Top-level input loop.                                                   .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void handleinput(void){
	int c;

	//if(currmode == PM_NORMAL){
	if((c = getch()) != ERR){

		// Repeat
		if(isdigit(c)){
			if(!cmdrepeat){
				cmdrepeat = 1;
				cmdrepeatnum = _char2int(c);
			}else{
				cmdrepeatnum = (cmdrepeatnum*10) + _char2int(c);
			}
		}else{
			normalmode(c);
		}
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < char nextchar() >                                                     .|
///  Wait for the next keyboard char and return it.                          .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
char nextchar(void){
	char ch;
	ch = getch();
	while (ch == ERR){
		ch = getch();
		if(ch != ERR )
			return ch;
		usleep(10000);
	}
	return ch;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void setdisplay(char*) >                                              .|
///  Sets a message to be popped up for a while.                             .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void setdisplay(char *str){
	disptick = 350;
	dispmesg = str;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void drawgui() >                                                      .|
///  Draw all text and graphix to the screen.                                .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void drawgui(){
	char buf[1024];
	int lines = LINES;
	int songcols[] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22};
	int trackcols[] = {0, 2, 4, 5, 7, 8, 9, 11, 12, 13};
	int instrcols[] = {0, 2, 3};
	u8 tempo;

	erase();
	attrset(A_UNDERLINE);
	mvaddstr(0, 0, "PINEAPPLEtRACKER");
	attrset(A_NORMAL);

	// display track num
	mvaddch(0, 31, ACS_ULCORNER);
	snprintf(buf, sizeof(buf), "%02x{}", currtrack);
	mvaddstr(0, 32, buf);
	drawtracked(29, 1, lines - 2);

	// display instrument num
	mvaddch(0, 51, ACS_ULCORNER);
	snprintf(buf, sizeof(buf), "%02x[]", currinstr);
	mvaddstr(0, 52, buf);
	drawinstred(49, 1, lines - 2);

	mvaddstr(1, 0, "Song");
	drawsonged(0, 1, lines - 2);

	// just a wild guess here..
	tempo = callbacktime * (-1) + 300;
	// display tempo
	mvaddch(0, 17, ACS_DEGREE);
	snprintf(buf, sizeof(buf), "%d()", tempo);
	mvaddstr(0, 18, buf);

	// display octave
	mvaddch(0, 24, ACS_PI);
	snprintf(buf, sizeof(buf), "%d<>", octave);
	mvaddstr(0, 25, buf);

	// display step amount
	mvaddstr(0, 60, "step -=");
	snprintf(buf, sizeof(buf), "%0x", step);
	mvaddstr(0, 68, buf);

	// display comment
	mvaddstr(2, 60, "comment:");
	snprintf(buf, sizeof(buf), "%s", comment);
	mvaddstr(3, 60, buf);

	if(currmode == PM_NORMAL){
		mvaddstr(getmaxy(stdscr)-1, 0, filename);
		if(!saved && currmode != PM_INSERT){
			addstr(" [+]");
			infinitemsg = NULL;
		}
	}

	if(disptick > 0){
		_display();
		disptick--;
	}

	if(currmode == PM_INSERT){
		infinitemsg = NULL;

		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr)-1, 0, "-- INSERT --");
	}else if(currmode == PM_VISUAL){
		infinitemsg = NULL;

		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr)-1, 0, "-- VISUAL --");
	}else if(currmode == PM_VISUALLINE){
		infinitemsg = NULL;

		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr)-1, 0, "-- VISUAL LINE --");
	}else if(currmode == PM_JAMMER){
		infinitemsg = NULL;

		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr)-1, 0, "-- JAMMER --");
	}else if(currmode == PM_CMDLINE){
		infinitemsg = NULL;

		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr) - 1, 0, cmdstr);
	}else if(infinitemsg != NULL){
		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr) - 1, 0, infinitemsg);
	}

	switch(currtab){
		case 0:
			move(1 + songy - songoffs, 0 + 4 + songcols[songx]);
			break;
		case 1:
			move(1 + tracky - trackoffs, 29 + 4 + trackcols[trackx]);
			break;
		case 2:
			move(1 + instry - instroffs, 49 + 4 + instrcols[instrx]);
			break;
	}

	refresh();

	if(disptick > 0){
		disptick--;
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void *spin_input(void *) >                                            .|
/// Pass this function to a thread to get the next key without blocking the  .\
\\\ the gui.                                                                 .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void *spin_input(void *tid){
	for(;;){
		handleinput();
		usleep(1000);
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void guiloop() >                                                      .|
/// guiloop() starts a new thread for the keyboard input, while running the  .\
\\\ gui loop in the main thread.                                             .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
//TODO: figure out a method where we don't have to use usleep()... the
// screen also jitters sometimes
void guiloop(void){
	int rc;
	pthread_t inputthread;
	pthread_attr_t attr;

	// make it detached ... we won't need to join it
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	rc = pthread_create(&inputthread,&attr,spin_input,(void *)0);

	if(rc)
		setdisplay("Uh oh!!! thread please");

#ifndef WINDOWS
	// don't treat the escape key like a meta key
	ESCDELAY = 50;
#endif

	//TODO: should check when the screen needs updating
	for(;;){
		drawgui();
		usleep(1000);
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  Internal functions                                                      .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void _display() >                                                     .|
///  Display dispmesg in the center of the screen.                           .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
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

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void drawsonged(int,int,int) >                                        .|
///  Draws the song editor.                                                  .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void drawsonged(int x, int y, int height){
	int i, j;
	char buf[1024];
	//NODE *match;

	if(songy < songoffs) songoffs = songy;
	if(songy >= songoffs + height) songoffs = songy - height + 1;

	for(i = 0; i < songlen; i++){
		if(i >= songoffs && i - songoffs < height){
			move(y + i - songoffs, x + 0);
			if(i == songy) attrset(A_BOLD);

			snprintf(buf, sizeof(buf), "%02x", i);

			if(i == 0){ addch(ACS_ULCORNER); }
			else if(i == songlen-1){ addch(ACS_LLCORNER); }
			else if(i%4 == 0){ addch(ACS_LTEE); }
			else if(i < songlen-1){ addch(ACS_VLINE); }
			addch(' ');

			// should this line be highlighted?
			//if( (match = list_contains(highlightlines, findu8, &i)) ){
			if( currtab == 0 && currmode == PM_VISUALLINE &&
				((i <= highlight_firstline && i >= highlight_lastline)
				|| (i >= highlight_firstline && i <= highlight_lastline)) ){
				attrset(A_REVERSE);
			}

			addstr(buf);
			for(j = 0; j < 4; j++){
				snprintf(buf, sizeof(buf), "%02x:%02x", song[i].track[j], song[i].transp[j]);
				addstr(buf);
				if(j != 3) addch(' ');
			}
			if(playsong && songpos == (i + 1)){
				attrset(A_STANDOUT);
				addch('*');
			}
			attrset(A_NORMAL);
		}
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void drawtracked(int,int,int) >                                       .|
///  Draws the track editor.                                                 .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void drawtracked(int x, int y, int height){
	u8 i, j;
	char buf[1024];

	if(tracky < trackoffs) trackoffs = tracky;
	if(tracky >= trackoffs + height) trackoffs = tracky - height + 1;

	for(i = 0; i < tracklen; i++){
		if(i >= trackoffs && i - trackoffs < height){
			move(y + i - trackoffs, x + 0);
			if(i == tracky) attrset(A_BOLD);

			snprintf(buf, sizeof(buf), "%02x", i);
			addstr(buf);

			if(i == 0){ addch(ACS_LLCORNER); }
			else if(i == 1){ addch(ACS_ULCORNER); }
			else if(i == tracklen-1){ addch(ACS_LLCORNER); }
			else if(i%4 == 0){ addch(ACS_LTEE); }
			else if(i < tracklen-1){ addch(ACS_VLINE); }
			addch(' ');

			// should this line be highlighted?
			//if( (match = list_contains(highlightlines, findu8, &i)) ){
			if(currtab == 1 && currmode == PM_VISUALLINE
				&& ((i <= highlight_firstline && i >= highlight_lastline)
				|| (i >= highlight_firstline && i <= highlight_lastline)) ){
				attrset(A_REVERSE);
			}

			if (currtab == 1 && currmode == PM_VISUAL)
				attrset(A_REVERSE);

			if(track[currtrack].line[i].note){
				snprintf(buf, sizeof(buf), "%s%d",
					notenames[(track[currtrack].line[i].note - 1) % 12],
					(track[currtrack].line[i].note - 1) / 12);
			}else{
				snprintf(buf, sizeof(buf), "---");
			}
			addstr(buf);
			snprintf(buf, sizeof(buf), " %02x", track[currtrack].line[i].instr);
			addstr(buf);
			for(j = 0; j < 2; j++){
				if(track[currtrack].line[i].cmd[j]){
					snprintf(buf, sizeof(buf), " %c%02x",
						track[currtrack].line[i].cmd[j],
						track[currtrack].line[i].param[j]);
				}else{
					snprintf(buf, sizeof(buf), " ...");
				}
				addstr(buf);
			}
			if(playtrack && ((i + 1) % tracklen) == trackpos){
				attrset(A_STANDOUT);
				addch('*');
			}
			attrset(A_NORMAL);
		}
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void drawinstred(int,int,int) >                                       .|
///  Draws the instrument editor.                                            .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void drawinstred(int x, int y, int height){
	u8 i;
	char buf[1024];

	if(instry >= instrument[currinstr].length) instry = instrument[currinstr].length - 1;

	if(instry < instroffs) instroffs = instry;
	if(instry >= instroffs + height) instroffs = instry - height + 1;

	for(i = 0; i < instrument[currinstr].length; i++){
		if(i >= instroffs && i - instroffs < height){
			move(y + i - instroffs, x + 0);
			if(i == instry) attrset(A_BOLD);

			snprintf(buf, sizeof(buf), "%02x", i);
			addstr(buf);

			if(i == 0){ addch(ACS_LLCORNER); }
			else if(i == 1){ addch(ACS_ULCORNER); }
			else if(i == instrument[currinstr].length-1){ addch(ACS_LLCORNER); }
			else if(i < instrument[currinstr].length-1){ addch(ACS_VLINE); }
			addch(' ');

			// should this line be highlighted?
			//if( (match = list_contains(highlightlines, findu8, &i)) ){
			if( currtab == 2 && currmode == PM_VISUALLINE &&
				((i <= highlight_firstline && i >= highlight_lastline)
				|| (i >= highlight_firstline && i <= highlight_lastline)) ){
				attrset(A_REVERSE);
			}

			snprintf(buf, sizeof(buf), "%c ", instrument[currinstr].line[i].cmd);
			addstr(buf);
			if(instrument[currinstr].line[i].cmd == '+' || instrument[currinstr].line[i].cmd == '='){
				if(instrument[currinstr].line[i].param){
					snprintf(buf, sizeof(buf), "%s%d",
						notenames[(instrument[currinstr].line[i].param - 1) % 12],
						(instrument[currinstr].line[i].param - 1) / 12);
				}else{
					snprintf(buf, sizeof(buf), "---");
				}
			}else{
				snprintf(buf, sizeof(buf), "%02x", instrument[currinstr].line[i].param);
			}
			addstr(buf);
			attrset(A_NORMAL);
		}
	}
}
