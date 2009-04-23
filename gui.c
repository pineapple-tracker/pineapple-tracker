/* vi:set ts=4 sts=4 sw=4: */

/* welcome to gui.c, enjoy your stay 8-) */

#include "stuff.h"
#include "gui.h"

/*                  */
// ** LOCAL VARS ** //
/*                  */
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

static char filename[1024];

static char *notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-"};

/*                       */
// ** LOCAL FUNCTIONS ** //
/*                       */
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
static void _display(void);
void _setdisplay(char *str);

/*                              */
// ** END LOCAL DECLARATIONS ** //
/*                              */

int currmode = PM_NORMAL;
int octave = 4;
int songlen = 1;
int tracklen = TRACKLEN;
int currtrack = 1;
int currinstr = 1;
int currtab = 0;

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


static int _hexdigit(char c){
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

/* _hexinc and _hexdec wrap around */
static int _hexinc(int x){
	return (x >= 0 && x <= 14)? x+1 : 0;
}
static int _hexdec(int x){
	return (x >= 1 && x <= 15)? x-1 : 15;
}

static int _nextfreetrack(){
	int skiptherest = 0;

	for(int i = 1; i <= 0xff; i++){
		for(int j = 0; j < tracklen; j++){
			if(track[i].line[j].note) skiptherest = 1;
			for(int k = 0; k < 2; k++){
				if(track[i].line[j].cmd[k]) skiptherest = 1;
				if(track[i].line[j].param[k]) skiptherest = 1;
			}

			// skip the rest of this track?
			if(skiptherest){
				skiptherest = 0;
				break;
			}

			// this track is free, so return the index
			if(j == tracklen-1) return i;
		}
	}

	_setdisplay("_nextfreetrack() failed somehow..");
	return -1;
}

static int _nextfreeinstr(){
	for(int i = 1; i <= 0xff; i++){
		if(instrument[i].line[0].cmd == '0')
			return i;
	}

	_setdisplay("_nextfreeinstr() failed somehow..");
	return -1;
}

/* Wait for the next keyboard char and return it.
 * This stops the screen from being updated. */
static char _nextchar(){
	char ch;
	ch = getch();
	while (ch == ERR){
		ch = getch();
		if(ch != ERR ){
			return ch;
		}
		usleep(10000);
	}
	return ch;
}

static int _char2int(char ch){
	if(isdigit(ch)){
		return (int)ch - '0';
	}
	return -1;
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

void _initsonglines(void){
	for(int i=0; i < songlen; i++){
		memmove(&song[i + 0], &song[i + 1], sizeof(struct songline) * (songlen - i - 1));
		if(i < 4){
			song[0].track[i] = 0x000;
			song[0].transp[i] = 0x000;
		}
	}
	songlen = 1;
}

void _inittracks(void){
	for(int i=0; i < 256; i++){
		for(int j=0; j < TRACKLEN; j++){
			track[i].line[j].note = 0x0000;
			track[i].line[j].instr = 0x0000;
			for(int k=0; k < 2; k++){
				track[i].line[j].cmd[k] = 0x0000;
				track[i].line[j].param[k] = 0x0000;
			}
		}
	}
}

void _initinstrs(void){
	for(int i=1; i < 256; i++){
		instrument[i].length = 1;
		instrument[i].line[0].cmd = '0';
		instrument[i].line[0].param = 0;
	}
}

void readsong(int pos, int ch, u8 *dest){ 
	dest[0] = song[pos].track[ch];
	dest[1] = song[pos].transp[ch];
}

void readtrack(int num, int pos, struct trackline *tl){
	tl->note = track[num].line[pos].note;
	tl->instr = track[num].line[pos].instr;
	tl->cmd[0] = track[num].line[pos].cmd[0];
	tl->cmd[1] = track[num].line[pos].cmd[1];
	tl->param[0] = track[num].line[pos].param[0];
	tl->param[1] = track[num].line[pos].param[1];
}

void readinstr(int num, int pos, u8 *il){
	if(pos >= instrument[num].length){
		il[0] = 0;
		il[1] = 0;
	}else{
		il[0] = instrument[num].line[pos].cmd;
		il[1] = instrument[num].line[pos].param;
	}
}

void savefile(char *fname){
	FILE *f;
	int i, j;

	f = fopen(fname, "w");
	if(!f){
		fprintf(stderr, "save error!\n");
		return;
	}

	fprintf(f, "musicchip tune\n");
	fprintf(f, "version 1\n");
	fprintf(f, "\n");
	fprintf(f, "tempo: %d\n", callbacktime);
	for(i = 0; i < songlen; i++){
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
	for(i = 1; i < 256; i++){
		for(j = 0; j < tracklen; j++){
			struct trackline *tl = &track[i].line[j];

			if(tl->note || tl->instr || tl->cmd[0] || tl->cmd[1]){
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
	for(i = 1; i < 256; i++){
		if(instrument[i].length > 1){
			for(j = 0; j < instrument[i].length; j++){
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

int loadfile(char *fname){
	FILE *f;
	char buf[1024];
	int cmd[3];
	int i1, i2, trk[4], transp[4], param[3], note, instr;
	int i;

	snprintf(filename, sizeof(filename), "%s", fname);

	f = fopen(fname, "r");
	if(!f){
		return -1;
	}

	songlen = 1;
	while(!feof(f) && fgets(buf, sizeof(buf), f)){
		if(1 == sscanf(buf, "tempo: %hhd", &callbacktime)){
			callbacktime = (u8)callbacktime;
		}else if(9 == sscanf(buf, "songline %x %x %x %x %x %x %x %x %x",
			&i1,
			&trk[0],
			&transp[0],
			&trk[1],
			&transp[1],
			&trk[2],
			&transp[2],
			&trk[3],
			&transp[3])){

			for(i = 0; i < 4; i++){
				song[i1].track[i] = trk[i];
				song[i1].transp[i] = transp[i];
			}
			if(songlen <= i1) songlen = i1 + 1;
		}else if(8 == sscanf(buf, "trackline %x %x %x %x %x %x %x %x",
			&i1,
			&i2,
			&note,
			&instr,
			&cmd[0],
			&param[0],
			&cmd[1],
			&param[1])){

			track[i1].line[i2].note = note;
			track[i1].line[i2].instr = instr;
			for(i = 0; i < 2; i++){
				track[i1].line[i2].cmd[i] = cmd[i];
				track[i1].line[i2].param[i] = param[i];
			}
		}else if(4 == sscanf(buf, "instrumentline %x %x %x %x",
			&i1,
			&i2,
			&cmd[0],
			&param[0])){

			instrument[i1].line[i2].cmd = cmd[0];
			instrument[i1].line[i2].param = param[0];
			if(instrument[i1].length <= i2) instrument[i1].length = i2 + 1;
		}
	}

	fclose(f);
	return 0;
}

void exitgui(){
	endwin();
}

void initgui(){
	initscr();

	//if(setlocale(LC_CTYPE,"en_US.utf8") != NULL) _setdisplay("UTF-8 enabled!");

	// don't send newline on Enter key, and don't echo chars to the screen.
	nonl();
	noecho();

	// make sure behaviour for special keys like ^H isn't overridden
	keypad(stdscr, FALSE);

	// nodelay() makes getch() non-blocking. This will cause the cpu to spin
	// whenever we use getch() in a loop. This is necessary so the screen will
	// update when you aren't pressing keys. halfdelay()'s minimum timeout time
	// is one tenth of a second, which is too long for our purposes.
	//
	// Right now we are calling usleep() whenever we use getch() in a loop so
	// the cpu won't spin. This solution isn't the best, for two reasons:
	//    1. We're still wasting a little bit of cpu!!!!!
	//    2. It is possible to enter keys faster than the usleep time. It's
	//       especially easy to do this by setting your key repeat rate really
	//       high and moving up or down, and the screen will lag a little.
	// 
	// Because of these two small problems, maybe we should eventually use
	// keyboard interrupts to trigger gui events. I haven't done any research
	// on that yet.
	nodelay(stdscr, TRUE);

	_initinstrs();

	atexit(exitgui);
}

void drawsonged(int x, int y, int height){
	int i, j;
	char buf[1024];

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

void drawtracked(int x, int y, int height){
	int i, j;
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

void drawinstred(int x, int y, int height){
	int i;
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

void optimize(){
	u8 used[256], replace[256];
	int i, j;

	memset(used, 0, sizeof(used));
	for(i = 0; i < songlen; i++){
		for(j = 0; j < 4; j++){
			used[song[i].track[j]] = 1;
		}
	}

	j = 1;
	replace[0] = 0;
	for(i = 1; i < 256; i++){
		if(used[i]){
			replace[i] = j;
			j++;
		}else{
			replace[i] = 0;
		}
	}

	for(i = 1; i < 256; i++){
		if(replace[i] && replace[i] != i){
			memcpy(&track[replace[i]], &track[i], sizeof(struct track));
		}
	}

	for(i = 0; i < songlen; i++){
		for(j = 0; j < 4; j++){
			song[i].track[j] = replace[song[i].track[j]];
		}
	}

	for(i = 1; i < 256; i++){
		u8 last = 255;

		for(j = 0; j < tracklen; j++){
			if(track[i].line[j].instr){
				if(track[i].line[j].instr == last){ track[i].line[j].instr = 0; }else{ last = track[i].line[j].instr;
				}
			}
		}
	}
}

static FILE *exportfile = 0;
static int exportbits = 0;
static int exportcount = 0;
static int exportseek = 0;

void putbit(int x){
	if(x){
		exportbits |= (1 << exportcount);
	}
	exportcount++;
	if(exportcount == 8){
		if(exportfile){
			fprintf(exportfile, "\t.byte\t0x%02x\n", exportbits);
		}
		exportseek++;
		exportbits = 0;
		exportcount = 0;
	}
}

void exportchunk(int data, int bits){
	int i;

	for(i = 0; i < bits; i++){
		putbit(!!(data & (1 << i)));
	}
}

int alignbyte(){
	if(exportcount){
		if(exportfile){
			fprintf(exportfile, "\t.byte\t0x%02x\n", exportbits);
		}
		exportseek++;
		exportbits = 0;
		exportcount = 0;
	}
	if(exportfile) fprintf(exportfile, "\n");
	return exportseek;
}

int packcmd(u8 ch){
	if(!ch) return 0;
	if(strchr(validcmds, ch)){
		return strchr(validcmds, ch) - validcmds;
	}
	return 0;
}

void exportdata(FILE *f, int maxtrack, int *resources){
	int i, j;
	int nres = 0;

	exportfile = f;
	exportbits = 0;
	exportcount = 0;
	exportseek = 0;

	for(i = 0; i < 16 + maxtrack; i++){
		exportchunk(resources[i], 13);
	}

	resources[nres++] = alignbyte();

	for(i = 0; i < songlen; i++){
		for(j = 0; j < 4; j++){
			if(song[i].transp[j]){
				exportchunk(1, 1);
				exportchunk(song[i].track[j], 6);
				exportchunk(song[i].transp[j], 4);
			}else{
				exportchunk(0, 1);
				exportchunk(song[i].track[j], 6);
			}
		}
	}

	for(i = 1; i < 16; i++){
		resources[nres++] = alignbyte();

		if(instrument[i].length > 1){
			for(j = 0; j < instrument[i].length; j++){
				exportchunk(packcmd(instrument[i].line[j].cmd), 8);
				exportchunk(instrument[i].line[j].param, 8);
			}
		}

		exportchunk(0, 8);
	}

	for(i = 1; i <= maxtrack; i++){
		resources[nres++] = alignbyte();

		for(j = 0; j < tracklen; j++){
			u8 cmd = packcmd(track[i].line[j].cmd[0]);

			exportchunk(!!track[i].line[j].note, 1);
			exportchunk(!!track[i].line[j].instr, 1);
			exportchunk(!!cmd, 1);

			if(track[i].line[j].note){
				exportchunk(track[i].line[j].note, 7);
			}

			if(track[i].line[j].instr){
				exportchunk(track[i].line[j].instr, 4);
			}

			if(cmd){
				exportchunk(cmd, 4);
				exportchunk(track[i].line[j].param[0], 8);
			}
		}
	}
}

void export(){
	FILE *f = fopen("exported.s", "w");
	FILE *hf = fopen("exported.h", "w");
	int i, j;
	int maxtrack = 0;
	int resources[256];

	exportfile = 0;
	exportbits = 0;
	exportcount = 0;
	exportseek = 0;

	for(i = 0; i < songlen; i++){
		for(j = 0; j < 4; j++){
			if(maxtrack < song[i].track[j]) maxtrack = song[i].track[j];
		}
	}

	fprintf(f, "\t.global\tsongdata\n\n");

	fprintf(hf, "#define MAXTRACK\t0x%02x\n", maxtrack);
	fprintf(hf, "#define SONGLEN\t\t0x%02x\n", songlen);

	fprintf(f, "songdata:\n");

	exportdata(0, maxtrack, resources);

	fprintf(f, "# ");
	for(i = 0; i < 16 + maxtrack; i++){
		fprintf(f, "%04x ", resources[i]);
	}
	fprintf(f, "\n");

	exportdata(f, maxtrack, resources);

	fclose(f);
	fclose(hf);
}

/*void initjoystick(){
	SDL_Event event;
	SDL_Joystick *joystick = NULL;

	SDL_JoystickEventState(SDL_ENABLE);
	if(SDL_NumJoysticks > 0){
		joystick = SDL_JoystickOpen(0);
	}

	sdlmainloop(event, joystick);
}

void sdlmainloop(SDL_Event event, SDL_Joystick *joystick){
	SDL_JoystickEventState(SDL_ENABLE);
	joystick = SDL_JoystickOpen(0);

	// wtf does this do
	while(SDL_PollEvent(&event)){ //nik its just checking if a bit is set :) 
	//while(!sdl_finished){       //   oh hmmm.. ok.
		switch(event.type){  
			case SDL_KEYDOWN:
				break;

			case SDL_JOYBUTTONDOWN:
				switch(event.jbutton.button){
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
}*/

/* actions are anything that can be repeated by entering a number beforehand */
/*enum {
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
	ACT_ADDLINE,
	ACT_DELLINE,
	ACT_CLRONETHING,
	ACT_CLRITALL,

	// song view
	ACT_TRACKINC,
	ACT_TRACKDEC,
	ACT_TRANSPINC,
	ACT_TRANSPDEC,

	// phrase view
	ACT_NOTEINC,
	ACT_NOTEDEC,
	ACT_OCTAVEINC,
	ACT_OCTAVEDEC,
	ACT_INSTRINC,
	ACT_INSTRDEC,
	ACT_FXINC,
	ACT_FXDEC,
	ACT_PARAMINC,
	ACT_PARAMDEC
};
*/

/* execute an action */
int z;
char currcmd;

void insertc (int c){
	int x;

	x = _hexdigit(c);
	if(x >= 0){
		if(currtab == 2
		&& instrx > 0
		&& instrument[currinstr].line[instry].cmd != '+'
		&& instrument[currinstr].line[instry].cmd != '='){
			switch(instrx){
				case 1: SETHI(instrument[currinstr].line[instry].param, x); break;
				case 2: SETLO(instrument[currinstr].line[instry].param, x); break;
			}
		}
		if(currtab == 1 && trackx > 1){
			switch(trackx){
				case 2: SETHI(track[currtrack].line[tracky].instr, x); break;
				case 3: SETLO(track[currtrack].line[tracky].instr, x); break;
				case 5: if(track[currtrack].line[tracky].cmd[0])
					SETHI(track[currtrack].line[tracky].param[0], x); break;
				case 6: if(track[currtrack].line[tracky].cmd[0])
					SETLO(track[currtrack].line[tracky].param[0], x); break;
				case 8: if(track[currtrack].line[tracky].cmd[1])
					SETHI(track[currtrack].line[tracky].param[1], x); break;
				case 9: if(track[currtrack].line[tracky].cmd[1])
					SETLO(track[currtrack].line[tracky].param[1], x); break;
			}
		}
		if(currtab == 0){
			switch(songx & 3){
				case 0: SETHI(song[songy].track[songx / 4], x); break;
				case 1: SETLO(song[songy].track[songx / 4], x); break;
				case 2: SETHI(song[songy].transp[songx / 4], x); break;
				case 3: SETLO(song[songy].transp[songx / 4], x); break;
			}
		}
	}
	x = freqkey(c);
	if(x >= 0){
		if(currtab == 2
		&& instrx
		&& (instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '=')){
			instrument[currinstr].line[instry].param = x;
		}
		if(currtab == 1 && !trackx){
			track[currtrack].line[tracky].note = x;
			if(x){
				track[currtrack].line[tracky].instr = currinstr;
			}else{
				track[currtrack].line[tracky].instr = 0;
			}
			if(x) iedplonk(x, currinstr);
		}
	}
	if(currtab == 2 && instrx == 0){
		if(strchr(validcmds, c)){
			instrument[currinstr].line[instry].cmd = c;
		}
	}
	if(currtab == 1 && (trackx == 4 || trackx == 7)){
		if(strchr(validcmds, c)){
			if(c == '.' || c == '0') c = 0;
			track[currtrack].line[tracky].cmd[(trackx - 3) / 3] = c;
		}
	}
}

void act_mvleft(void){
	switch(currtab){
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
}

void act_mvright(void){
	switch(currtab){
		case 0:
			if(songx < 15) songx++;
			break;
		case 1:
			if(trackx < 9) trackx++;
			break;
		case 2:
			if(instrx < 2) instrx++;
			break;
	}
}

void act_mvup(void){
	switch(currtab){
		case 0:
			if(songy){
				songy--;
			}else{
				songy = 0;
			}
			break;
		case 1:
			if(tracky){
				tracky--;
			}else{
				tracky = 0;
			}
			break;
		case 2:
			if(instry){
				instry--;
			}else{
				instry = 0;
			}
			break;
	}
}

void act_mvdown(void){
	switch(currtab){
		case 0:
			if(songy < songlen - 1){
				songy++;
			}else{
				songy = songlen - 1;
			}
			break;
		case 1:
			if(tracky < tracklen - 1){
				tracky++;
			}else{
				tracky = tracklen - 1;
			}
			break;
		case 2:
			if(instry < instrument[currinstr].length - 1){
				instry++;
			}else{
				instry = instrument[currinstr].length - 1;
			}
			break;
	}
}

void act_bigmvup(void){
	switch(currtab){
		case 0:
			if(songy >= 8){
				songy -= 8;
			}else{
				songy = 0;
			}
			break;
		case 1:
			if(tracky >= 8){
				tracky -= 8;
			}else{
				tracky = 0;
			}
			break;
		case 2:
			if(instry >= 8) instry -= 8;
			break;
	}
}

void act_bigmvdown(void){
	switch(currtab){
		case 0:
			if(songy < songlen - 8){
				songy += 8;
			}else{
				songy = songlen - 1;
			}
			break;
		case 1:
			if(tracky < tracklen - 8){
				tracky += 8;
			}else{
				tracky = tracklen - 1;
			}
			break;
		case 2:
			if(instry < instrument[currinstr].length - 8) instry += 8;
			break;
	}
}

void act_viewphraseinc(void){
	if(currtrack < 0xff){
		currtrack++;
	}else if(currtrack == 0xff){
		currtrack = 1;
	}
	if(playtrack){
		startplaytrack(currtrack);
	}
}

void act_viewphrasedec(void){
	if(currtrack > 1){
		currtrack--;
	}else if(currtrack == 1){
		currtrack = 0xff;
	}
	if(playtrack){
		startplaytrack(currtrack);
	}
}

void act_viewinstrinc(void){
	if(currinstr == 0xff) currinstr = 0x01;
	else currinstr++;
}

void act_viewinstrdec(void){
	if(currinstr == 1) currinstr = 0xff;
	else currinstr--;
}

void act_trackinc(void){
	if( (songx%2)==0 ){
		if(songx >= 240 && songx < 256){
			song[songy].track[songx/4] -= 240;
		}else{
			song[songy].track[songx/4] += 16;
		}
	}else{
		if( (song[songy].track[songx/4] % 16) == 15){
			song[songy].track[songx/4] -= 15;
		}else{
			song[songy].track[songx/4]++;
		}
	}
}

void act_trackdec(void){
	if( (songx%2)==0 ){
		if(songx <= 15 && songx >= 0){
			song[songy].track[songx/4] += 240;
		}else{
			song[songy].track[songx/4] -= 16;
		}
	}else{
		if( (song[songy].track[songx/4] % 16) == 0){
			song[songy].track[songx/4] += 15;
		}else{
			song[songy].track[songx/4]--;
		}
	}
}

void act_transpinc(void){
	if( (songx%2)==0 ){
		if(songx >= 240 && songx < 256){
			song[songy].transp[songx/4] -= 240;
		}else{
			song[songy].transp[songx/4] += 16;
		}
	}else{
		if( (song[songy].transp[songx/4] % 16) == 15){
			song[songy].transp[songx/4] -= 15;
		}else{
			song[songy].transp[songx/4]++;
		}
	}
}

void act_transpdec(void){
	if( (songx%2)==0 ){
		if(songx <= 15 && songx >= 0){
			song[songy].transp[songx/4] += 240;
		}else{
			song[songy].transp[songx/4] -= 16;
		}
	}else{
		if( (song[songy].transp[songx/4] % 16) == 0){
			song[songy].transp[songx/4] += 15;
		}else{
			song[songy].transp[songx/4]--;
		}
	}
}

void act_noteinc(void){
	if(currtab==1){
		// if current note < H7
		if( track[currtrack].line[tracky].note < 96 ){
			track[currtrack].line[tracky].note++;
		}else{
			track[currtrack].line[tracky].note = 0;
		}
	}else if(currtab==2){
		if( instrument[currinstr].line[instry].param < 96 ){
			instrument[currinstr].line[instry].param++;
		}else{
			instrument[currinstr].line[instry].param = 0;
		}
	}
}

void act_notedec(void){
	if(currtab==1){
		if( track[currtrack].line[tracky].note > 0 ){
			track[currtrack].line[tracky].note--;
		}else{
			track[currtrack].line[tracky].note = 96;
		}
	}else if(currtab==2){
		if( instrument[currinstr].line[instry].param > 0 ){
			instrument[currinstr].line[instry].param--;
		}else{
			instrument[currinstr].line[instry].param = 96;
		}
	}
}

// TODO: make octaveinc/dec wrap around
void act_octaveinc(void){
	if(currtab==1){
		if( track[currtrack].line[tracky].note+12 < 97 ){
			track[currtrack].line[tracky].note+=12;
		}else{
			track[currtrack].line[tracky].note = 96;
		}
	}else if(currtab==2){ if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
			if( instrument[currinstr].line[instry].param+12 < 97 ){
				instrument[currinstr].line[instry].param+=12;
			}else{
				instrument[currinstr].line[instry].param = 96;
			}
		}
	}
}

void act_octavedec(void){
	if(currtab==1){
		if( track[currtrack].line[tracky].note-12 > 0 ){
			track[currtrack].line[tracky].note-=12;
		}else{
			track[currtrack].line[tracky].note = (unsigned long)NULL;
		}
	}else if(currtab==2){
		if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
			if( instrument[currinstr].line[instry].param-12 > 0 ){
				instrument[currinstr].line[instry].param-=12;
			}else{
				instrument[currinstr].line[instry].param = (unsigned long)NULL;
			}
		}
	}
}

void act_instrinc(void){
	switch(trackx){
		case 2:
			SETHI(track[currtrack].line[tracky].instr,
					_hexinc(track[currtrack].line[tracky].instr >> 4) );
			break;
		case 3:
			SETLO(track[currtrack].line[tracky].instr,
					_hexinc(track[currtrack].line[tracky].instr & 0x0f) );
			break;
	}
}

void act_instrdec(void){
	switch(trackx){
		case 2:
			SETHI(track[currtrack].line[tracky].instr,
					_hexdec(track[currtrack].line[tracky].instr >> 4) );
			break;
		case 3:
			SETLO(track[currtrack].line[tracky].instr,
					_hexdec(track[currtrack].line[tracky].instr & 0x0f) );
			break;
	}
}

void act_fxinc(void){
	if(currtab==1){
		currcmd = track[currtrack].line[tracky].cmd[trackx % 2];
		// there must be a better way to do this...
		if((unsigned long)currcmd == (unsigned long)NULL){
			track[currtrack].line[tracky].cmd[trackx % 2] = validcmds[0];
		}else{
			for(z = 0; z < strlen(validcmds); z++){
				if(currcmd == validcmds[z]){
					if(z == (strlen(validcmds)-1)){
						track[currtrack].line[tracky].cmd[trackx % 2] = (unsigned long)NULL;
					}else{
						track[currtrack].line[tracky].cmd[trackx % 2] = validcmds[z+1];
					}
					continue;
				}
			}
		}
	}else if(currtab==2){
		currcmd = instrument[currinstr].line[instry].cmd;
		for(z = 0; z < strlen(validcmds); z++){
			if(currcmd == validcmds[z]){
				if(z == (strlen(validcmds)-1)){
					instrument[currinstr].line[instry].cmd = validcmds[0];
				}else{
					instrument[currinstr].line[instry].cmd = validcmds[z+1];
				}
				continue;
			}
		}
	}
}

void act_fxdec(void){
	if(currtab==1){
		currcmd = track[currtrack].line[tracky].cmd[trackx % 2];
		if((unsigned long)currcmd == (unsigned long)NULL){
			track[currtrack].line[tracky].cmd[trackx % 2] = validcmds[strlen(validcmds)-1];
		}else{
			for(z = 0; z < strlen(validcmds); z++){
				if(currcmd == validcmds[z]){
					if(z==0){
						track[currtrack].line[tracky].cmd[trackx % 2] = (unsigned long)NULL;
					}else{
						track[currtrack].line[tracky].cmd[trackx % 2] = validcmds[z-1];
					}
					continue;
				}
			}
		}
	}else if(currtab==2){
		currcmd = instrument[currinstr].line[instry].cmd;
		for(z = 0; z < strlen(validcmds); z++){
			if(currcmd == validcmds[z]){
				if(z==0){
					instrument[currinstr].line[instry].cmd = validcmds[strlen(validcmds)-1];
				}else{
					instrument[currinstr].line[instry].cmd = validcmds[z-1];
				}
				continue;
			}
		}
	}
}

void act_paraminc(void){
	if(currtab==1){
		if(trackx==5 || trackx==8){
			SETHI(track[currtrack].line[tracky].param[(trackx - 1) % 2],
					_hexinc(track[currtrack].line[tracky].param[(trackx - 1) % 2] >> 4) );
			return;
		}else if(trackx==6 || trackx==9){
			SETLO(track[currtrack].line[tracky].param[trackx % 2],
					_hexinc(track[currtrack].line[tracky].param[trackx % 2] & 0x0f) );
			return;
		}
	}else if(currtab == 2){
		if(instrx == 1){
			SETHI(instrument[currinstr].line[instry].param,
					_hexinc(instrument[currinstr].line[instry].param >> 4) );
			return;
		}else if(instrx == 2){
			SETLO(instrument[currinstr].line[instry].param,
					_hexinc(instrument[currinstr].line[instry].param & 0x0f) );
			return;
		}
	}
}

void act_paramdec(void){
	if(currtab==1){
		if(trackx==5 || trackx==8){
			SETHI(track[currtrack].line[tracky].param[(trackx-1) % 2],
					_hexdec(track[currtrack].line[tracky].param[(trackx-1) % 2] >> 4) );
			return;
		}else if(trackx==6 || trackx==9){
			SETLO(track[currtrack].line[tracky].param[trackx % 2],
					_hexdec(track[currtrack].line[tracky].param[trackx % 2] & 0x0f) );
			return;
		}
	}else if(currtab == 2){
		if(instrx == 1){
			SETHI(instrument[currinstr].line[instry].param,
					_hexdec(instrument[currinstr].line[instry].param >> 4) );
			return;
		}else if(instrx == 2){
			SETLO(instrument[currinstr].line[instry].param,
					_hexdec(instrument[currinstr].line[instry].param & 0x0f) );
			return;
		}
	}
}

void act_addline(void){
	if(currtab == 2){
		struct instrument *in = &instrument[currinstr];

		if(in->length < 256){
			memmove(&in->line[instry + 2], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
			instry++;
			in->length++;
			in->line[instry].cmd = '0';
			in->line[instry].param = 0;
		}
	}else if(currtab == 0){
		if(songlen < 256){
			memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
			songy++;
			songlen++;
			memset(&song[songy], 0, sizeof(struct songline));
		}
	}
}

void act_delline(void){
	if(currtab == 2){
		struct instrument *in = &instrument[currinstr];

		if(in->length > 1){
			memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
			in->length--;
			if(instry >= in->length) instry = in->length - 1;
		}
	}else if(currtab == 0){
		if(songlen > 1){
			memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
			songlen--;
			if(songy >= songlen) songy = songlen - 1;
		}
	}
}

void act_clronething(void){
	if(currtab == 0){
		if( (songx%4) < 2){
			if( (songx%2)==0 ){
				song[songy].track[songx/4] = (song[songy].track[songx/4] - song[songy].track[songx/4]) + song[songy].track[songx/4]%16;
			}else{
				song[songy].track[songx/4] -= song[songy].track[songx/4]%16;
			}
		}else{
			if( (songx%2)==0 ){
				song[songy].transp[songx/4] = (song[songy].transp[songx/4] - song[songy].transp[songx/4]) + song[songy].transp[songx/4]%16;
			}else{
				song[songy].transp[songx/4] -= song[songy].transp[songx/4]%16;
			}
		}
		//memcpy(&tclip, &song[songy], sizeof(struct songline));
	}else if(currtab == 1){
		switch(trackx){
			case 0:
				memcpy(&tclip, &track[currtrack].line[tracky], sizeof(struct trackline));
				track[currtrack].line[tracky].note = 0;
				track[currtrack].line[tracky].instr = 0;
				//memmove
				break;
			case 2:
				memcpy(&tclip, &track[currtrack].line[tracky].instr, sizeof(struct trackline));
				SETHI(track[currtrack].line[tracky].instr, 0);
				break;
			case 3:
				memcpy(&tclip, &track[currtrack].line[tracky].instr, sizeof(struct trackline));
				SETLO(track[currtrack].line[tracky].instr, 0);
				break;
			case 4:
				track[currtrack].line[tracky].cmd[0] = 0;
				break;
			case 5:
				SETHI(track[currtrack].line[tracky].param[0],0);
				break;
			case 6:
				SETLO(track[currtrack].line[tracky].param[0],0);
				break;
			case 7:
				track[currtrack].line[tracky].cmd[1] = 0;
				break;
			case 8:
				SETHI(track[currtrack].line[tracky].param[1],0);
				break;
			case 9:
				SETLO(track[currtrack].line[tracky].param[1],0);
				break;
			default:
				_setdisplay("in ACT_CLRONETHING");
				break;
		}
	}else if(currtab == 2){
		if(instrx == 0){
			instrument[currinstr].line[instry].cmd = '0';
		}else if(instrx == 1){
			if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
				instrument[currinstr].line[instry].param = 0;
			}else{
				SETHI(instrument[currinstr].line[instry].param,0);
			}
		}else if(instrx == 2){
			if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
				instrument[currinstr].line[instry].param = 0;
			}else{
				SETLO(instrument[currinstr].line[instry].param,0);
			}
		}
	}
}

void act_clritall(void){
	if(currtab == 0){
		int ci;
		for(ci = 0; ci < 4; ci++){
			song[songy].track[ci] = 0;
			song[songy].transp[ci] = 0;
		}
	}else if(currtab == 1){
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
	}else if(currtab == 2){
		instrument[currinstr].line[instry].cmd = '0';
		instrument[currinstr].line[instry].param = 0;
	}
}

/* vi insert mode */
void insertmode(){
	int c;
	currmode = PM_INSERT;
	drawgui();
	for(;;){
		if((c = getch()) != ERR) switch(c){
			case KEY_ESCAPE:
				currmode = PM_NORMAL;
				guiloop();
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
			/* change octave */
			case '<':
				if(octave) octave--;
				break;
			case '>':
				if(octave < 8) octave++;
				break;
			/* change instrument */
			case CTRL('J'):
				if(currtab == 2){
					act_viewinstrdec();
				}else if(currtab == 1){
					act_viewphrasedec();
				}
				break;
			case CTRL('K'):
				if(currtab == 2){
					act_viewinstrinc();
				}else if(currtab == 1){
					act_viewphraseinc();
				}
				break;
			case '[':
				act_viewinstrdec();
				break;
			case ']':
				act_viewinstrinc();
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
				c = _nextchar();
				switch(c){
					case 'Z':
						savefile(filename);
						erase();
						refresh();
						endwin();
						exit(0);
						break;
					case 'Q':
						erase();
						refresh();
						endwin();
						exit(0);
						break;
				}
				break;
			case ' ':
				silence();
				currmode = PM_NORMAL;
				guiloop();
				break;
			case ENTER:
				if(currtab != 2){
					if(currtab == 1){
						silence();
						startplaytrack(currtrack);
					}else if(currtab == 0){
						silence();
						startplaysong(songy);
					}
				}
				break;
			case '`':
				if(currtab == 0){
					int t = song[songy].track[songx / 4];
					if(t) currtrack = t;
					currtab = 1;
				}else if(currtab == 1){
					currtab = 0;
				}
				break;
			default:
				insertc(c);
				if(currtab == 1){
					tracky++;
					tracky %= tracklen;
				}else if(currtab == 2){
					if(instry < instrument[currinstr].length-1) instry++;
					instry %= instrument[currinstr].length;
				}
				saved = 0;
		}
		drawgui();
		usleep(10000);
	}
}

void parsecmd(char cmd[]){
	//if(cmd[1] == 'w'){
	//switch(strcmp(cmd, 
	if(strcmp(cmd, ":w") == 0){
		savefile(filename);
		saved = 1;
	}else if(strcmp(cmd, ":q") == 0){
		erase();
		refresh();
		endwin();
		exit(0);
	}else if(strcmp(cmd, ":write") == 0){
		savefile(filename);
		saved = 1;
	}else if(strcmp(cmd, ":wq") == 0){
		savefile(filename);
		saved = 1;
		erase();
		refresh();
		endwin();
		exit(0);
	}else if(strcmp(cmd, ":quit") == 0){
		erase();
		refresh();
		endwin();
		exit(0);
	}else if(cmd[1]=='e' && cmd[2]==' '){
		// if the file doesn't exist, clear the song
		if(loadfile(cmd+3)){
			_initsonglines();
			_inittracks();
			_initinstrs();
		}
	}else if(isdigit(cmd[1])){
		int gotoline = atoi(cmd+1);

		switch(currtab){
			case 0:
				if(gotoline>songlen){ songy=songlen-1; }
				else{ songy = gotoline; }
				break;
			case 1:
				if(gotoline>tracklen){ tracky=tracklen-1; }
				else{ tracky = gotoline; }
				break;
			case 2:
				if(gotoline>instrument[currinstr].length){
					instry=instrument[currinstr].length-1; }
				else{ instry = gotoline; }
				break;
		}
	}else 
		_setdisplay("not a tracker command!");
	return;
}

/* vi cmdline mode */
void cmdlinemode(){
	u16 c;
	keypad(stdscr, TRUE);

	currmode = PM_CMDLINE;
	strncat(cmdstr, ":", 50);
	for(;;){
		drawgui();

		c = _nextchar();
		switch(c){
			case KEY_ESCAPE:
				//cmdstr = "";
				currmode = PM_NORMAL;
				goto end;
			case ENTER:
				parsecmd(cmdstr);
				goto end;
#ifndef WINDOWS
			case 0x07:
				_setdisplay("\\o/");
				cmdstr[strlen(cmdstr)-1] = '\0';
				break;
#endif
			case '\t':
				break;
			default:
				strncat(cmdstr, &c, 50);
				break;
		}
	}
end:
	strcpy(cmdstr, "");
	keypad(stdscr, FALSE);
	return;
}

/* jammer mode */
void jammermode(void){
	int c, x;
	currmode = PM_JAMMER;
	while(currmode == PM_JAMMER){
		if((c = getch()) != ERR) switch(c){
			case KEY_ESCAPE:
				currmode = PM_NORMAL;
				break;
			case '[':
				act_viewinstrdec();
				break;
			case ']':
				act_viewinstrinc();
				break;
			case '<':
				if(octave) octave--;
				break;
			case '>':
				if(octave < 8) octave++;
				break;
			default:
				x = freqkey(c);

				if(x > 0){
					iedplonk(x, currinstr);
				}

				break;
		}
		drawgui();
		usleep(10000);
	}
}

/* visual mode */
void visualmode(void){
	int c;
	currmode = PM_VISUAL;
	attrset(A_REVERSE);
	while(currmode == PM_VISUAL){
		if((c = getch()) != ERR) switch(c){
			case KEY_ESCAPE:
				currmode = PM_NORMAL;
				break;
		}
		drawgui();
	}
	attrset(A_BOLD);
	return;
}

/* normal mode */
void executekey(int c){
	int i;

	// don't save the action if it's a movement or a repeat
	if(c != 'h' &&
		c != 'j' && 
		c != 'k' && 
		c != 'l' && 
		c != CTRL('D') && 
		c != CTRL('U') && 
		c != CTRL('H') && 
		c != CTRL('L') && 
		c != 'g' && 
		c != 'G' && 
		c != '.'){
		lastaction = c;
		lastrepeat = cmdrepeatnum;
	}

	for(i=0; i<cmdrepeatnum; i++){
		switch(c){
		/* add line */
		case 'a':
			if(currtab == 2){
				struct instrument *in = &instrument[currinstr];

				if(in->length < 256){
					memmove(&in->line[instry + 2], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
					instry++;
					in->length++;
					in->line[instry].cmd = '0';
					in->line[instry].param = 0;
				}
			}else if(currtab == 0){
				if(songlen < 256){
					memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
					songy++;
					songlen++;
					memset(&song[songy], 0, sizeof(struct songline));
				}
			}
			break;
		case '.':
			cmdrepeatnum = lastrepeat;
			executekey(lastaction);
			break;
		case KEY_ESCAPE:
			disptick = 0;
			break;
		case CTRL('Y'):
			switch(currtab){
				case 0:
					if(songoffs>0){
						if(songy==getmaxy(stdscr)-3+songoffs)
							songy--;
						songoffs--;
					}
					break;
				case 1:
					if(trackoffs>0){
						if(tracky==getmaxy(stdscr)-3+trackoffs)
							tracky--;
						trackoffs--;
					}
					break;
				case 2:
					if(instroffs>0){
						if(instry==getmaxy(stdscr)-3+instroffs)
							instry--;
						instroffs--;
					}
					break;
			}
			break;
		case CTRL('E'):
			switch(currtab){
				case 0:
					if(songy<=songlen-2){
						if(songy==songoffs)
							songy++;
						songoffs++;
					}
					break;
				case 1:
					if(tracky<=tracklen-2){
						if(tracky==trackoffs)
							tracky++;
						trackoffs++;
					}
					break;
				case 2:
					if(instry<=instrument[currinstr].length-2){
						if(instry==instroffs)
							instry++;
						instroffs++;
					}
					break;
			}
			break;
		case 'H':
			switch(currtab){
				case 0:
					songy = songoffs;
					break;
				case 1:
					tracky = trackoffs;
					break;
				case 2:
					instry = instroffs;
					break;
			}
			break;
		case 'M':
			switch(currtab){
				case 0:
					songy = (songlen+1<getmaxy(stdscr)-2)?
							songlen/2 : (getmaxy(stdscr)-2+songoffs)/2;
					break;
				case 1:
					tracky = (tracklen+1<getmaxy(stdscr)-2)?
							tracklen/2 : (getmaxy(stdscr)-2+trackoffs)/2;
					break;
				case 2:
					instry = (instrument[currinstr].length-1<getmaxy(stdscr)-2)?
							(instrument[currinstr].length-1)/2 : (getmaxy(stdscr)-2+instroffs)/2;
					break;
			}
			break;
		case 'L':
			switch(currtab){
				case 0:
					songy = (songlen+1<getmaxy(stdscr)-2)?
							songlen-1 : getmaxy(stdscr)-3+songoffs;
					break;
				case 1:
					tracky = (tracklen+1<getmaxy(stdscr)-2)?
							tracklen-1 : getmaxy(stdscr)-3+trackoffs;
					break;
				case 2:
					instry = (instrument[currinstr].length-1<getmaxy(stdscr)-2)?
							instrument[currinstr].length-1 : getmaxy(stdscr)-3+instroffs;
					break;
			}
			break;
		case 'g':
			if(_nextchar() == 'g'){
				switch(currtab){
					case 0:
						songy = 0;
						break;
					case 1:
						tracky = 0;
						break;
					case 2:
						instry = 0;
						break;
				}
			}
			break;
		case 'G':
			switch(currtab){
				case 0:
					songy = songlen - 1;
					break;
				case 1:
					tracky = tracklen - 1;
					break;
				case 2:
					instry = instrument[currinstr].length - 1;
					break;
			}
			break;

		// yank
		case 'y':
			c = _nextchar();
			switch(c){
				case 'y':
					//tclip = malloc(1);
					if(currtab == 0){
						tcliplen = 1;
						memcpy(&tclip, &song[songy], sizeof(struct songline));
					}else if(currtab == 1){
						tcliplen = 1;
						memcpy(&tclip, &track[currtrack].line[tracky], sizeof(struct trackline));
					}else if(currtab == 2){
						icliplen = 1;
						memcpy(&iclip, &instrument[currinstr].line[instry], sizeof(struct instrline));
					}
					break;
				case 'j':
					//tclip = malloc(2);
					if(currtab == 0){
						tcliplen = 2;
						memcpy(&tclip[0], &song[songy], sizeof(struct songline));
						act_mvdown();
						memcpy(&tclip[1], &song[songy], sizeof(struct songline));
					}else if(currtab == 1){
						tcliplen = 2;
						memcpy(&tclip[0], &track[currtrack].line[tracky], sizeof(struct trackline));
						act_mvdown();
						memcpy(&tclip[1], &track[currtrack].line[tracky], sizeof(struct trackline));
					}else if(currtab == 2){
						icliplen = 2;
						memcpy(&iclip[0], &instrument[currinstr].line[instry], sizeof(struct instrline));
						act_mvdown();
						memcpy(&iclip[1], &instrument[currinstr].line[instry], sizeof(struct instrline));
					}
					break;
				case 'k':
					//tclip = malloc(2);
					if(currtab == 0){
						tcliplen = 2;
						memcpy(&tclip[1], &song[songy], sizeof(struct songline));
						act_mvup();
						memcpy(&tclip[0], &song[songy], sizeof(struct songline));
					}else if(currtab == 1){
						tcliplen = 2;
						memcpy(&tclip[1], &track[currtrack].line[tracky], sizeof(struct trackline));
						act_mvup();
						memcpy(&tclip[0], &track[currtrack].line[tracky], sizeof(struct trackline));
					}else if(currtab == 2){
						icliplen = 2;
						memcpy(&iclip[1], &instrument[currinstr].line[instry], sizeof(struct instrline));
						act_mvup();
						memcpy(&iclip[0], &instrument[currinstr].line[instry], sizeof(struct instrline));
					}
					break;
			}
			break;

		//paste
		case 'p':
			if(currtab == 0){
				if(songlen < 256){
					// insert new line
					memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
					songy++;
					songlen++;
					memset(&song[songy], 0, sizeof(struct songline));

					// paste to new line
					memcpy(&song[songy], &tclip, sizeof(struct songline));
				}
			}else if(currtab == 1){
					for(int i = 0; i < tcliplen; i++){
						memcpy(&track[currtrack].line[tracky], &tclip[i], sizeof(struct trackline));
						if(tracky < tracklen-1) tracky++;
					}
			}else if(currtab == 2){
				if(instrument[currinstr].length < 256){
					// insert new line
					struct instrument *in = &instrument[currinstr];

					instry++;
					memmove(&in->line[instry + 1], &in->line[instry + 0], sizeof(struct instrline) * (in->length - instry));
					in->length++;
					in->line[instry].cmd = '0';
					in->line[instry].param = 0;

					// paste to new line
					memcpy(&instrument[currinstr].line[instry], &iclip, sizeof(struct instrline));
				}
				//if(instry < instrument[currinstr].length-1) instry++;
			}
			break;

		// copy everything in the current phrase or instrument into the next free one
		case '^':
			if(currtab == 1){
				f = _nextfreetrack();
				memcpy(&track[f], &track[currtrack], sizeof(struct track));
				currtrack = f;
			}else if(currtab == 2){
				f = _nextfreeinstr();
				memcpy(&instrument[f], &instrument[currinstr], sizeof(struct instrument));
				currinstr = f;
			}
			break;

		// TODO: Y and P can be removed after we make visual mode
		// copy whole phrase or instrument
		case 'Y':
			if(currtab == 1){
				memcpy(&tclip, &track[currtrack], sizeof(struct track));
			}else if(currtab == 2){
				memcpy(&iclip, &instrument[currinstr], sizeof(struct instrument));
			}
			break;
		// paste whole phrase or instrument
		case 'P':
			if(currtab == 1){
				memcpy(&track[currtrack], &tclip, sizeof(struct track));
			}else if(currtab == 2){
				memcpy(&instrument[currinstr], &iclip, sizeof(struct instrument));
			}
			break;

		/* delete line */
		// TODO: clean this SHIT up
		// TODO: add an ACT_ function for delete
		case 'd':
			c = _nextchar();
			switch(c){
				case 'd':
					if(currtab == 2){
						struct instrument *in = &instrument[currinstr];

						if(in->length > 1){
							memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
							in->length--;
							if(instry >= in->length) instry = in->length - 1;
						}
					}else if(currtab == 0){
						if(songlen > 1){
							memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
							songlen--;
							if(songy >= songlen) songy = songlen - 1;
						}
					}
					break;
				case 'k':
					if(currtab == 2){
						struct instrument *in = &instrument[currinstr];
						instry--;
						int i;
						for(i=0; i<2; i++){
							if(in->length > 1){
								memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
								in->length--;
								if(instry >= in->length) instry = in->length - 1;
							}
						}
					}else if(currtab == 0){
						songy--;
						int i;
						for(i=0; i<2; i++){
							if(songlen > 1){
								memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
								songlen--;
								if(songy >= songlen) songy = songlen - 1;
							}
						}
					}
					break;
				case 'j':
					if(currtab == 2){
						struct instrument *in = &instrument[currinstr];

						int i;
						for(i=0; i<2; i++){
							if(in->length > 1){
								memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
								in->length--;
								if(instry >= in->length) instry = in->length - 1;
							}
						}
					}else if(currtab == 0){
						int i;
						for(i=0; i<2; i++){
							if(songlen > 1){
								memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
								songlen--;
								if(songy >= songlen) songy = songlen - 1;
							}
						}
					}
					break;
			}
			break;
		/* Clear */
		case 'x':
			act_clronething();
			break;
		case 'X':
			act_clritall();
			break;
		case ENTER:
			if(currtab != 2){
				if(currtab == 1){
					silence();
					startplaytrack(currtrack);
				}else if(currtab == 0){
					silence();
					startplaysong(songy);
				}
			}
			break;
		case 'Z':
			c = _nextchar();
			switch(c){
				case 'Z':
					savefile(filename);
					erase();
					refresh();
					endwin();
					exit(0);
					break;
				case 'Q':
					erase();
					refresh();
					endwin();
					exit(0);
					break;
			}
			break;
		/* Enter command mode */
		case ':':
			cmdlinemode();
			break;
		case ' ':
			silence();
			break;
		case '`':
			if(currtab == 0){
				int t = song[songy].track[songx / 4];
				if(t) currtrack = t;
				currtab = 1;
			}else if((currtab == 1) && ((trackx == 2) || (trackx == 3))){
				int i = track[currtrack].line[tracky].instr;
				if(i) currinstr = i;
				currtab = 2;
			}	else if(currtab == 1){
				currtab = 0;
			}else if(currtab == 2){
				currtab = 1;
			}
			break;
		/* Enter insert mode */
		case 'i':
			insertmode();
			break;
		/* Enter visual mode */
		case 'v':
			visualmode();
			break;
		/* enter jammer mode */
		case CTRL('A'):
			jammermode();
			break;
		/* Add new line and enter insert mode */
		case 'o':
			if(currtab == 2){
				struct instrument *in = &instrument[currinstr];

				if(in->length < 256){
					memmove(&in->line[instry + 2], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
					instry++;
					in->length++;
					in->line[instry].cmd = '0';
					in->line[instry].param = 0;
				}
			}else if(currtab == 0){
				if(songlen < 256){
					memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
					songy++;
					songlen++;
					memset(&song[songy], 0, sizeof(struct songline));
				}
			}
			insertmode();
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
		case '<':
			if(octave) octave--;
			break;
		case '>':
			if(octave < 8) octave++;
			break;
		case '{':
			if(currtrack > 1) currtrack--;
			break;
		case '}':
			if(currtrack < 255) currtrack++;
			break;
		case 'J':
			if(currtab == 0){
				if( (songx%4) < 2){
					act_trackdec();
				}else{
					act_transpdec();
				}
			}else if(currtab == 1){
				switch(trackx){
					case 0:
						act_notedec();
						break;
					case 1:
						act_octavedec();
						break;
					case 2:
						act_instrdec();
						break;
					case 3:
						act_instrdec();
						break;
					case 4:
						act_fxdec();	
						break;
					case 5:
					case 6:
						act_paramdec();	
						break;
					case 7:
						act_fxdec();	
						break;
					case 8:
					case 9:
						act_paramdec();	
						break;
					default:
						_setdisplay("in J");
						break;
					}
			}else if(currtab == 2){
				switch(instrx){
					case 0:
						act_fxdec();	
						break;
					case 1:
						if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
							act_notedec();
						}else{
							act_paramdec();	
						}
						break;
					case 2:
						if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
							act_notedec();
						}else{
							act_paramdec();	
						}
						break;
				}
			}
			break;
		case 'K':
			if(currtab == 0){
				if( (songx%4) < 2){
					act_trackinc();
				}else{
					act_transpinc();
				}
			}else if(currtab == 1){
				switch(trackx){
					case 0:
						act_noteinc();
						break;
					case 1:
						act_octaveinc();	
						break;
					case 2:
						act_instrinc();
						break;
					case 3:
						act_instrinc();
						break;
					case 4:
						act_fxinc();	
						break;
					case 5:
					case 6:
						act_paraminc();	
						break;
					case 7:
						act_fxinc();	
						break;
					case 8:
					case 9:
						act_paraminc();	
						break;
					default:
						_setdisplay("in K");
						break;
				}
			}else if(currtab == 2){
				switch(instrx){
					case 0:
						act_fxinc();	
						break;
					case 1:
						if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
							act_noteinc();
						}else{
							act_paraminc();	
						}
						break;
					case 2:
						if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
							act_noteinc();
						}else{
							act_paraminc();	
						}
						break;
				}
			}
			break;
		case CTRL('J'):
			if(currtab == 2){
				act_viewinstrdec();
			}else if(currtab == 1){
				act_viewphrasedec();
			}
			break;
		case CTRL('K'):
			if(currtab == 2){
				act_viewinstrinc();
			}else if(currtab == 1){
				act_viewphraseinc();
			}
			break;
		case '[':
			act_viewinstrdec();
			break;
		case ']':
			act_viewinstrinc();
			break;
		case '(':
			callbacktime++;
			break;
		case ')':
			callbacktime--;
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
		case CTRL('U'):
			act_bigmvup();
			break;
		case CTRL('D'):
			act_bigmvdown();
			break;
		/*case CTRL('P'):
			vimode = false;
			break;*/

		// replace
		case 'r':
			insertc(_nextchar());
			break;

		default:
			break;
		} // end switch
	} // end for
	cmdrepeatnum = 1;
	cmdrepeat = 0;
}

/* main input loop */
void handleinput(){
	int c;

	/*if(currmode == PM_NORMAL){*/
	if((c = getch()) != ERR){

		/* Repeat */
		if(isdigit(c)){
			if(!cmdrepeat){
				cmdrepeat = 1;
				cmdrepeatnum = _char2int(c);
			}else{
				cmdrepeatnum = (cmdrepeatnum*10) + _char2int(c);
			}
		}else{
			executekey(c);
		}
	}
	/* linus' commands */
	/*}else{
		if((c = getch()) != ERR) switch(c){
			case 10:
			case 13:
				if(currtab != 2){
					if(currtab == 1){
						startplaytrack(currtrack);
					}else if(currtab == 0){
						startplaysong(songy);
					}
				}
				break;
			case CTRL('P'):
				currmode = PM_NORMAL;
				break;
			case ' ':
				silence();
				if(currmode == PM_NORMAL){
					currmode = PM_INSERT;
				}else{
					currmode = PM_NORMAL;
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
				setdisplay("*saved*");
				saved = true;
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
				if(currtab == 0){
					int t = song[songy].track[songx / 4];
					if(t) currtrack = t;
					currtab = 1;
				}else if(currtab == 1){
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
				act_mvleft();
				break;
			case KEY_DOWN:
				act_mvdown();
				break;
			case KEY_UP:
				act_mvup();
				break;
			case KEY_RIGHT:
				act_mvright();
				break;
			case 'C':
				if(currtab == 2){
					memcpy(&iclip, &instrument[currinstr], sizeof(struct instrument));
				}else if(currtab == 1){
					memcpy(&tclip, &track[currtrack], sizeof(struct track));
				}
				break;
			case 'V':
				if(currtab == 2){
					memcpy(&instrument[currinstr], &iclip, sizeof(struct instrument));
				}else if(currtab == 1){
					memcpy(&track[currtrack], &tclip, sizeof(struct track));
				}
				break;
			default:
				if(currmode == PM_INSERT){
					x = _hexdigit(c);
					if(x >= 0){
						if(currtab == 2
						&& instrx > 0
						&& instrument[currinstr].line[instry].cmd != '+'
						&& instrument[currinstr].line[instry].cmd != '='){
							switch(instrx){
								case 1: SETHI(instrument[currinstr].line[instry].param, x); break;
								case 2: SETLO(instrument[currinstr].line[instry].param, x); break;
							}
						}
						if(currtab == 1 && trackx > 0){
							switch(trackx){
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
						if(currtab == 0){
							switch(songx & 3){
								case 0: SETHI(song[songy].track[songx / 4], x); break;
								case 1: SETLO(song[songy].track[songx / 4], x); break;
								case 2: SETHI(song[songy].transp[songx / 4], x); break;
								case 3: SETLO(song[songy].transp[songx / 4], x); break;
							}
						}
					}
					x = freqkey(c);
					if(x >= 0){
						if(currtab == 2
						&& instrx
						&& (instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '=')){
							instrument[currinstr].line[instry].param = x;
						}
						if(currtab == 1 && !trackx){
							track[currtrack].line[tracky].note = x;
							if(x){
								track[currtrack].line[tracky].instr = currinstr;
							}else{
								track[currtrack].line[tracky].instr = 0;
							}
							tracky++;
							tracky %= tracklen;
							if(x) iedplonk(x, currinstr);
						}
					}
					if(currtab == 2 && instrx == 0){
						if(strchr(validcmds, c)){
							instrument[currinstr].line[instry].cmd = c;
						}
					}
					if(currtab == 1 && (trackx == 3 || trackx == 6 || trackx == 9)){
						if(strchr(validcmds, c)){
							if(c == '.' || c == '0') c = 0;
							track[currtrack].line[tracky].cmd[(trackx - 3) / 3] = c;
						}
					}
					if(c == 'A'){
						if(currtab == 2){
							struct instrument *in = &instrument[currinstr];

							if(in->length < 256){
								memmove(&in->line[instry + 2], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
								instry++;
								in->length++;
								in->line[instry].cmd = '0';
								in->line[instry].param = 0;
							}
						}else if(currtab == 0){
							if(songlen < 256){
								memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
								songy++;
								songlen++;
								memset(&song[songy], 0, sizeof(struct songline));
							}
						}
					}else if(c == 'I'){
						if(currtab == 2){
							struct instrument *in = &instrument[currinstr];

							if(in->length < 256){
								memmove(&in->line[instry + 1], &in->line[instry + 0], sizeof(struct instrline) * (in->length - instry));
								in->length++;
								in->line[instry].cmd = '0';
								in->line[instry].param = 0;
							}
						}else if(currtab == 0){
							if(songlen < 256){
								memmove(&song[songy + 1], &song[songy + 0], sizeof(struct songline) * (songlen - songy));
								songlen++;
								memset(&song[songy], 0, sizeof(struct songline));
							}
						}
					}else if(c == 'D'){
						if(currtab == 2){
							struct instrument *in = &instrument[currinstr];

							if(in->length > 1){
								memmove(&in->line[instry + 0], &in->line[instry + 1], sizeof(struct instrline) * (in->length - instry - 1));
								in->length--;
								if(instry >= in->length) instry = in->length - 1;
							}
						}else if(currtab == 0){
							if(songlen > 1){
								memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
								songlen--;
								if(songy >= songlen) songy = songlen - 1;
							}
						}
					}
				}else if(currmode == PM_INSERT || currmode == PM_JAMMER){
					x = freqkey(c);

					if(x > 0){
						iedplonk(x, currinstr);
					}
				}
				break;
		}
	}*/
	usleep(10000);
}

void _setdisplay(char *str){
	disptick = 350;
	dispmesg = str;
}

// display dispmesg in the center of the screen
static void _display(void){
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

	if(currmode == PM_NORMAL){
		mvaddstr(getmaxy(stdscr)-1, 0, filename);
		if(!saved && currmode != PM_INSERT) addstr(" [+]");
	}

	mvaddstr(1, 0, "Song");
	drawsonged(0, 1, lines - 2);

	if(disptick > 0){
		_display();
		disptick--;
	}

	if(currmode == PM_INSERT){
		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr)-1, 0, "-- INSERT --");
	}else if(currmode == PM_VISUAL){
		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr)-1, 0, "-- VISUAL --");
	}else if(currmode == PM_JAMMER){
		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr)-1, 0, "-- JAMMER --");
	}else if(currmode == PM_CMDLINE){
		move(getmaxy(stdscr)-1,0);
		clrtoeol();
		mvaddstr(getmaxy(stdscr) - 1, 0, cmdstr);
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


void guiloop(){
#ifndef WINDOWS
	// don't treat the escape key like a meta key
	ESCDELAY = 50;
#endif
	for(;;){
		drawgui();
		handleinput();
	}
}

