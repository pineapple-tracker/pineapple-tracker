/* vi:set syntax= ts=8 sts=8 sw=8 noexpandtab: */
/* WARNING: this file is ROSS STYLE */

#include "pineapple.h"
#include "gui.h"
#include "filetypes.h"

#include <curses.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int f;
int tcliplen, icliplen = 0;
int lastinsert = 0;

int hexdigit(char c){
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

int nextfreetrack(){
	int skiptherest = 0;

	for(int i = 1; i <= 0xff; i++){
		for(int j = 0; j < tune->tracklen; j++){
			if(tune->trk[i].line[j].note) skiptherest = 1;
			for(int k = 0; k < 2; k++){
				if(tune->trk[i].line[j].cmd[k]) skiptherest = 1;
				if(tune->trk[i].line[j].param[k]) skiptherest = 1;
			}

			// skip the rest of this track?
			if(skiptherest){
				skiptherest = 0;
				break;
			}

			// this track is free, so return the index
			if(j == (tune->tracklen)-1) return i;
		}
	}

	setdisplay("nextfreetrack() failed somehow..");
	return -1;
}

int nextfreeinstr(){
	for(int i = 1; i <= 0xff; i++){
		if(instrument[i].line[0].cmd == '0')
			return i;
	}

	setdisplay("nextfreeinstr() failed somehow..");
	return -1;
}

void _insertc(int c){
	int x;

	x = hexdigit(c);
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
				case 2: SETHI(tune->trk[currtrack].line[tracky].instr, x); break;
				case 3: SETLO(tune->trk[currtrack].line[tracky].instr, x); break;
				case 5: if(tune->trk[currtrack].line[tracky].cmd[0])
					SETHI(tune->trk[currtrack].line[tracky].param[0], x); break;
				case 6: if(tune->trk[currtrack].line[tracky].cmd[0])
					SETLO(tune->trk[currtrack].line[tracky].param[0], x); break;
				case 8: if(tune->trk[currtrack].line[tracky].cmd[1])
					SETHI(tune->trk[currtrack].line[tracky].param[1], x); break;
				case 9: if(tune->trk[currtrack].line[tracky].cmd[1])
					SETLO(tune->trk[currtrack].line[tracky].param[1], x); break;
			}
		}
		if(currtab == 0){
			switch(songx & 3){
				case 0: SETHI(tune->sng[songy].track[songx / 4], x); break;
				case 1: SETLO(tune->sng[songy].track[songx / 4], x); break;
				case 2: SETHI(tune->sng[songy].transp[songx / 4], x); break;
				case 3: SETLO(tune->sng[songy].transp[songx / 4], x); break;
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
			tune->trk[currtrack].line[tracky].note = x;
			if(x){
				tune->trk[currtrack].line[tracky].instr = currinstr;
			}else{
				tune->trk[currtrack].line[tracky].instr = 0;
			}
			if(x) lft_iedplonk(x, currinstr);
		}
	}
	if(currtab == 2 && instrx == 0){
		if(strchr(validcmds, c))
			instrument[currinstr].line[instry].cmd = c;
	}
	if(currtab == 1 && (trackx == 4 || trackx == 7)){
		if(strchr(validcmds, c)){
			if(c == '.' || c == '0') c = 0;
			tune->trk[currtrack].line[tracky].cmd[(trackx - 3) / 3] = c;
		}
	}
	// for repeat
	lastinsert = c;
}

// _isnumber() determines if a string is a number
// returns 1 if the given arg is a number, 0 otherwise.
// for use with isdigit(), isxdigit(), etc...
int _isnumber(const char* str, int (*func) (int)){
	int i = 0;

	for(;;){
		if( func(str[i]) ){
			i++;
		}else if( str[i]=='\0' ){
			return 1;
		}else{
			return 0;
		}
	}
}

// Converts a hexadecimal string to integer
// return values:
//	0: conversion successful
//	1: string is empty
//	2: string has more than 8 bytes
//	4: Conversion is in process but abnormally terminated by
//		illegal hexadecimal character
// from http://devpinoy.org/blogs/cvega/archive/2006/06/19/xtoi-hex-to-integer-c-function.aspx
int xtoi(const char* xs, unsigned int* result){
	size_t szlen = strlen(xs);
	int i, xv, fact;

	if(szlen > 0){
		// Converting more than 32bit hexadecimal value?
		if (szlen>8) return 2; // exit

		// Begin conversion here
		*result = 0;
		fact = 1;

		// Run until no more character to convert
		for(i=szlen-1; i>=0 ;i--){
			if(isxdigit(*(xs+i))){
				if (*(xs+i)>=97){
					xv = ( *(xs+i) - 97) + 10;
				}else if( *(xs+i) >= 65){
					xv = (*(xs+i) - 65) + 10;
				}else{
					xv = *(xs+i) - 48;
				}
				*result += (xv * fact);
				fact *= 16;
			}else{
				// Conversion was abnormally terminated
				// by non hexadecimal digit, hence
				// returning only the converted with
				// an error value 4 (illegal hex character)
				return 4;
			}
		}
	}
	// Nothing to convert
	return 1;
}

void _parsecmd(char cmd[]){
	//if(cmd[1] == 'w'){
	//switch(strcmp(cmd,
	if(strcmp(cmd, ":w") == 0){
		lft_savefile(filename);
		saved = 1;
	}else if(strcmp(cmd, ":q") == 0){
		if(!saved){
			setdisplay("no write since last change! use :q! to override");
		}else{
			erase();
			refresh();
			endwin();
			exit(0);
		}
	}else if(strcmp(cmd, ":q!") == 0){
		erase();
		refresh();
		endwin();
		exit(0);
	}else if(strcmp(cmd, ":write") == 0){
		lft_savefile(filename);
		saved = 1;
	}else if(strcmp(cmd, ":wq") == 0 || strcmp(cmd, ":x") == 0){
		lft_savefile(filename);
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
		if(lft_loadfile(cmd+3)){
			initsonglines();
			inittracks();
			initinstrs();
		}
	//yucky if statement below.....probably better way to do it
	// maybe this is better??
	}else if(!strncmp(cmd+1,"save ",5)){
		lft_saveinstrument(cmd+6);
		setdisplay("d-_-b saved ins! d-_-b");
	}else if(!strncmp(cmd+1,"load ",5)){
		lft_loadinstrument(cmd+6);
		setdisplay("d-_-b loaded ins! d-_-b");
	}else if( _isnumber((char *)cmd+1,isxdigit) ){
		unsigned int goton = 1;
		xtoi(cmd+1,&goton);

		switch(currtab){
			case 0:
				songy = (goton>tune->songlen)? tune->songlen-1 : goton;
				break;
			case 1:
				currtrack = (goton>0xff)? 0xff : goton;
				break;
			case 2:
				currinstr = (goton>0xff)? 0xff : goton;
				break;
		}
	}else if(cmd[1] == 'c' && cmd[2] == ' '){
		strncpy(comment, cmd+3, sizeof(comment));
	}else 
		setdisplay("not a tracker command!");
	return;
}

/* normal mode */
void normalmode(int c){
	int i;

	// don't save the action for repeat if it's a movement or a repeat, or
	// something else that doesnt make sense to repeat
	if(c != 'h' &&
		c != 'j' && 
		c != 'k' && 
		c != 'l' && 
		c != CTRL('D') && 
		c != CTRL('U') && 
		c != CTRL('H') && 
		c != CTRL('L') && 
		c != 'H' && 
		c != 'M' && 
		c != 'L' && 
		c != 'g' && 
		c != 'G' && 
		c != '.'){
		lastaction = c;
		lastrepeatnum = cmdrepeatnum;
	}

	for(i=0; i<cmdrepeatnum; i++){
		switch(c){
		/* add line */
		case 'a':
			act_addline();
			break;
		case '.':
			// if the last command was a replace, just insert the last thing
			// inserted instead of calling insertmode()
			if(lastaction == 'r')
				_insertc(lastinsert);
			else
				normalmode(lastaction);
			cmdrepeatnum = lastrepeatnum;
			break;
		case KEY_ESCAPE:
			disptick = 0;
			jammermode();
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
					if(songy<=tune->songlen-2){
						if(songy==songoffs)
							songy++;
						songoffs++;
					}
					break;
				case 1:
					if(tracky<=(tune->tracklen)-2){
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

		// the second cases (to the right of the colon) for M and L
		// took some serious guesswork, so I'm not sure if they're
		// correct but they seem to work.
		case 'M':
			switch(currtab){
				case 0:
					songy = (tune->songlen <= getmaxy(stdscr)-2)?
							tune->songlen/2
							: ((getmaxy(stdscr)-6)/2) + songoffs;
					break;
				case 1:
					tracky = (tune->tracklen <= getmaxy(stdscr)-2)?
							tune->tracklen/2
							: ((getmaxy(stdscr)-6)/2) + trackoffs;
					break;
				case 2:
					instry = (instrument[currinstr].length <= getmaxy(stdscr)-2)?
							instrument[currinstr].length/2
							: ((getmaxy(stdscr)-6)/2) + instroffs;
					break;
			}
			break;
		case 'L':
			switch(currtab){
				case 0:
					songy = (tune->songlen <= getmaxy(stdscr)-2)?
							tune->songlen-1
							: getmaxy(stdscr)-3+songoffs;
					break;
				case 1:
					tracky = (tune->tracklen <= getmaxy(stdscr)-2)?
							tune->tracklen-1
							: getmaxy(stdscr)-3+trackoffs;
					break;
				case 2:
					instry = (instrument[currinstr].length <= getmaxy(stdscr)-2)?
							instrument[currinstr].length-1
							: getmaxy(stdscr)-3+instroffs;
					break;
			}
			break;
		case 'g':
			if(nextchar() == 'g'){
				act_mvtop();
			}
			break;
		case 'G':
			act_mvbottom();
			break;

		// yank
		case 'y':
			c = nextchar();
			switch(c){
				case 'y':
					//tclip = malloc(1);
					if(currtab == 0){
						tcliplen = 1;
						memcpy(&tclip, &tune->sng[songy], sizeof(struct songline));
					}else if(currtab == 1){
						tcliplen = 1;
						memcpy(&tclip, &tune->trk[currtrack].line[tracky], sizeof(struct trackline));
					}else if(currtab == 2){
						icliplen = 1;
						memcpy(&iclip, &instrument[currinstr].line[instry], sizeof(struct instrline));
					}
					break;
				case 'j':
					//tclip = malloc(2);
					if(currtab == 0){
						tcliplen = 2;
						memcpy(&tclip[0], &tune->sng[songy], sizeof(struct songline));
						act_mvdown();
						memcpy(&tclip[1], &tune->sng[songy], sizeof(struct songline));
					}else if(currtab == 1){
						tcliplen = 2;
						memcpy(&tclip[0], &tune->trk[currtrack].line[tracky], sizeof(struct trackline));
						act_mvdown();
						memcpy(&tclip[1], &tune->trk[currtrack].line[tracky], sizeof(struct trackline));
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
						memcpy(&tclip[1], &tune->sng[songy], sizeof(struct songline));
						act_mvup();
						memcpy(&tclip[0], &tune->sng[songy], sizeof(struct songline));
					}else if(currtab == 1){
						tcliplen = 2;
						memcpy(&tclip[1], &tune->trk[currtrack].line[tracky], sizeof(struct trackline));
						act_mvup();
						memcpy(&tclip[0], &tune->trk[currtrack].line[tracky], sizeof(struct trackline));
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
				if(tune->songlen < 256){
					for(int i = 0; i < tcliplen; i++){
						// insert new line
						memmove(&tune->sng[songy + 2], &tune->sng[songy + 1], sizeof(struct songline) * (tune->songlen - songy - 1));
						songy++;
						tune->songlen++;
						memset(&tune->sng[songy], 0, sizeof(struct songline));

						// paste to new line
						memcpy(&tune->sng[songy], &tclip[i], sizeof(struct songline));
					}
				}
			}else if(currtab == 1){
					for(int i = 0; i < tcliplen; i++){
						memcpy(&tune->trk[currtrack].line[tracky], &tclip[i], sizeof(struct trackline));
						if(tracky < (tune->tracklen)-step) tracky += step;
						else tracky = (tune->tracklen)-1;
					}
			}else if(currtab == 2){
				if(instrument[currinstr].length < 256){
					// insert new line
					for(int i = 0; i < icliplen; i++){
						struct instrument *in = &instrument[currinstr];

						instry++;
						memmove(&in->line[instry + 1], &in->line[instry + 0], sizeof(struct instrline) * (in->length - instry));
						in->length++;
						in->line[instry].cmd = '0';
						in->line[instry].param = 0;

						// paste to new line
						memcpy(&instrument[currinstr].line[instry], &iclip[i], sizeof(struct instrline));
					}
				}
				//if(instry < instrument[currinstr].length-1) instry++;
			}
			break;

		// copy everything in the current phrase or instrument into the next free one
		case '^':
			if(currtab == 1){
				f = nextfreetrack();
				memcpy(&tune->trk[f], &tune->trk[currtrack], sizeof(struct track));
				currtrack = f;
			}else if(currtab == 2){
				f = nextfreeinstr();
				memcpy(&instrument[f], &instrument[currinstr], sizeof(struct instrument));
				currinstr = f;
			}
			break;

		// TODO: Y and P can be removed after we make visual mode
		// copy whole phrase or instrument
		case 'Y':
			if(currtab == 1){
				memcpy(&tclip, &tune->trk[currtrack], sizeof(struct track));
			}else if(currtab == 2){
				memcpy(&iclip, &instrument[currinstr], sizeof(struct instrument));
			}
			break;
		// paste whole phrase or instrument
		case 'P':
			if(currtab == 1){
				memcpy(&tune->trk[currtrack], &tclip, sizeof(struct track));
			}else if(currtab == 2){
				memcpy(&instrument[currinstr], &iclip, sizeof(struct instrument));
			}
			break;

		/* delete line */
		// TODO: clean this SHIT up
		// TODO: add an ACT_ function for delete
		case 'd':
			c = nextchar();
			switch(c){
				case 'd':
					act_delline();
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
							if(tune->songlen > 1){
								memmove(&tune->sng[songy + 0], &tune->sng[songy + 1], sizeof(struct songline) * (tune->songlen - songy - 1));
								tune->songlen--;
								if(songy >= tune->songlen) songy = tune->songlen - 1;
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
							if(tune->songlen > 1){
								memmove(&tune->sng[songy + 0], &tune->sng[songy + 1], sizeof(struct songline) * (tune->songlen - songy - 1));
								tune->songlen--;
								if(songy >= tune->songlen) songy = tune->songlen - 1;
							}
						}
					}
					break;
			}
			break;
		/* undo */
		case 'u':
			act_undo();
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
			c = nextchar();
			switch(c){
				case 'Z':
					lft_savefile(filename);
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
		// TODO: make an act_ function for '`'
		case '`':
			if(currtab == 0){
				int t = tune->sng[songy].track[songx / 4];
				if(t) currtrack = t;
				currtab = 1;
				if(playtrack){
					startplaytrack(currtrack);
				}
			}else if((currtab == 1) && ((trackx == 2) || (trackx == 3))){
				int i = tune->trk[currtrack].line[tracky].instr;
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
		/* Enter visual line mode */
		case 'V':
			visuallinemode();
			break;
		/* enter jammer mode */
		case CTRL('A'):
			jammermode();
			break;
		/* Add new line and enter insert mode */
		case 'o':
			act_addline();
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
						setdisplay("in J");
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
						setdisplay("in K");
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
				act_viewtrackdec();
			}
			break;
		case CTRL('K'):
			if(currtab == 2){
				act_viewinstrinc();
			}else if(currtab == 1){
				act_viewtrackinc();
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
		case '-':
			if(step > 0) 
			  step--;
			break;
		case '=':
			if(step < 0x0f) 
			  step++;
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
			_insertc(nextchar());
			break;

		default:
			break;
		} // end switch
	} // end for
	cmdrepeatnum = 1;
	cmdrepeat = 0;
}

/* vi cmdline mode */
void cmdlinemode(void){
	char c;
	keypad(stdscr, TRUE);

	currmode = PM_CMDLINE;
	strncat(cmdstr, ":", 100);
	for(;;){
		drawgui();

		c = nextchar();
		switch(c){
			case KEY_ESCAPE:
				//cmdstr = "";
				currmode = PM_NORMAL;
				goto end;
			case ENTER:
				_parsecmd(cmdstr);
				goto end;
#ifndef WINDOWS
			case BACKSPACE:
				setdisplay("\\o/");
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

/* vi insert mode */
void insertmode(void){
	int c;
	currmode = PM_INSERT;
	drawgui();
	for(;;){
		if((c = getch()) != ERR) switch(c){
			case KEY_ESCAPE:
				currmode = PM_NORMAL;
				return;
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
					act_viewtrackdec();
				}
				break;
			case CTRL('K'):
				if(currtab == 2){
					act_viewinstrinc();
				}else if(currtab == 1){
					act_viewtrackinc();
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
				c = nextchar();
				switch(c){
					case 'Z':
						lft_savefile(filename);
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
				return;
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
					int t = tune->sng[songy].track[songx / 4];
					if(t) currtrack = t;
					currtab = 1;
				}else if(currtab == 1){
					currtab = 0;
				}
				break;
			default:
				_insertc(c);
				if(currtab == 1){
					tracky+=step;
					tracky %= (tune->tracklen);
				}else if(currtab == 2){
					//if(instry < instrument[currinstr].length-1) instry++;
					if(instrx < 2) instrx++;
					else instrx--;
					instry %= instrument[currinstr].length;
				}
				saved = 0;
		}
		drawgui();
		usleep(10000);
	}
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
					lft_iedplonk(x, currinstr);
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
	//attrset(A_REVERSE);
	if(currtab == 0){
	}else if(currtab == 1){
		highlight_firstx = trackx;
		highlight_lastx = trackx;
		highlight_firsty = tracky;
		highlight_lasty = tracky;
	}else if(currtab == 2){
	}else{
		highlight_firstx = -1;
		highlight_lastx = -1;
		highlight_firsty = -1;
		highlight_lasty = -1;
	}

	while(currmode == PM_VISUAL){
		if((c = getch()) != ERR) switch(c){
			case 'v':
			case KEY_ESCAPE:
				currmode = PM_NORMAL;
				break;
			case 'V':
				visuallinemode();
				break;
			case 'h':
				act_mvleft();
				if(currtab==0){
				}else if(currtab==1){
					highlight_lastx = trackx;
				}else if(currtab==2){
				}
				break;
			case 'j':
				act_mvdown();
				if(currtab==0){
				}else if(currtab==1){
					highlight_lasty = tracky;
				}else if(currtab==2){
				}
				break;
			case 'k':
				act_mvup();
				if(currtab==0){
				}else if(currtab==1){
					highlight_lasty = tracky;
				}else if(currtab==2){
				}
				break;
			case 'l':
				act_mvright();
				if(currtab==0){
				}else if(currtab==1){
					highlight_lastx = trackx;
				}else if(currtab==2){
				}
				break;
		}
		drawgui();
	}
	attrset(A_BOLD);
	return;
}

/* visual line mode */
void visuallinemode(void){
	int c;
	int min, max;
	char buf[1024];

	currmode = PM_VISUALLINE;

	/* Store the current line as the first and last node of a linked list */
	if(currtab==0){
		highlight_firstline = songy;
		highlight_lastline = songy;
	}else if(currtab==1){
		highlight_firstline = tracky;
		highlight_lastline = tracky;
	}else if(currtab==2){
		highlight_firstline = instry;
		highlight_lastline = instry;
	}else{
		highlight_firstline = -1;
		highlight_lastline = -1;
	}

	// initialize difference
	highlight_lineamount = 1;

	// make it visible to gui.c
	//highlightlines = firstnode;

	while(currmode == PM_VISUALLINE){
		if((c = getch()) != ERR) switch(c){
			case 'V':
			case KEY_ESCAPE:
				currmode = PM_NORMAL;
				break;
			case 'v':
				visualmode();
			case 'h':
				act_mvleft();
				break;
			case 'j':
				act_mvdown();
				// update lastnode
				if(currtab==0){
					highlight_lastline = songy;
				}else if(currtab==1){
					highlight_lastline = tracky;
				}else if(currtab==2){
					highlight_lastline = instry;
				}
				// update the highlighted length
				highlight_lineamount = (highlight_firstline>highlight_lastline)?
						highlight_firstline - highlight_lastline +1
						: highlight_lastline - highlight_firstline +1;
				break;
			case 'k':
				act_mvup();
				// update lastnode
				if(currtab==0){
					highlight_lastline = songy;
				}else if(currtab==1){
					highlight_lastline = tracky;
				}else if(currtab==2){
					highlight_lastline = instry;
				}
				// update the highlighted length
				highlight_lineamount = (highlight_firstline>highlight_lastline)?
						highlight_firstline - highlight_lastline +1
						: highlight_lastline - highlight_firstline +1;
				break;
			case 'l':
				act_mvright();
				break;
			case 'g':
				if(nextchar() == 'g'){
					act_mvtop();
				}
				break;
			case 'G':
				act_mvbottom();
				break;
			// d: copy every line that is highlighted to the paste buffer and clear them, too
			case 'd':
				min = (highlight_firstline < highlight_lastline)?
						highlight_firstline
						: highlight_lastline;
				max = (highlight_firstline < highlight_lastline)?
						highlight_lastline
						: highlight_firstline;
				if(currtab == 0){
					for(int i=min; i<=max; i++)
						act_clrinsongtab(i);
				}else if(currtab == 1){
					for(int i=min; i<=max; i++)
						act_clrintracktab(currtrack, i);
				}else if(currtab == 2){
					for(int i=min; i<=max; i++)
						act_clrininstrtab(currinstr, i);
				}
				//snprintf(buf, sizeof(buf), "%d fewer lines", highlight_lineamount);
				//infinitemsg = buf;
				currmode = PM_NORMAL;
				break;
			// y: copy every line that is highlighted to the paste buffer
			case 'y':
				if(currtab == 0){
					//tcliplen = 1;
					//memcpy(&tclip, &tune->sng[songy], sizeof(struct songline)*highlight_lineamount);
					tcliplen = highlight_lineamount;
					//moved up, then yanked
					if(highlight_firstline > highlight_lastline){
						for(int i = 0; i < highlight_lineamount; i++)
							memcpy(&tclip[i], &tune->sng[songy+i], sizeof(struct songline));
					//moved down, then yanked
					}else if(highlight_lastline > highlight_firstline){
						for(int i = highlight_lineamount-1, j = 0; i >= 0; i--, j++){
								memcpy(&tclip[i], &tune->sng[songy-j], sizeof(struct songline));
						}
					}
				}else if(currtab == 1){
					tcliplen = highlight_lineamount;
					//moved up, then yanked
					if(highlight_firstline > highlight_lastline){
						for(int i = 0; i < highlight_lineamount; i++)
							memcpy(&tclip[i], &tune->trk[currtrack].line[tracky+i], sizeof(struct trackline));
					//moved down, then yanked
					}else if(highlight_lastline > highlight_firstline){
						for(int i = highlight_lineamount-1, j = 0; i >= 0; i--, j++){
								memcpy(&tclip[i], &tune->trk[currtrack].line[tracky-j], sizeof(struct trackline));
						}
					}
				}else if(currtab == 2){
					//icliplen = 1;
					//memcpy(&iclip, &instrument[currinstr].line[instry], sizeof(struct instrline)*highlight_lineamount);
					icliplen = highlight_lineamount;
					//moved up, then yanked
					if(highlight_firstline > highlight_lastline){
						for(int i = 0; i < highlight_lineamount; i++)
							memcpy(&iclip[i], &instrument[currinstr].line[instry+i], sizeof(struct instrline));
					//moved down, then yanked
					}else if(highlight_lastline > highlight_firstline){
						for(int i = highlight_lineamount-1, j = 0; i >= 0; i--, j++){
								memcpy(&iclip[i], &instrument[currinstr].line[instry-j], sizeof(struct instrline));
						}
					}
				}

				snprintf(buf, sizeof(buf), "%d lines yanked", highlight_lineamount);
				infinitemsg = buf;
				currmode = PM_NORMAL;
				break;
		}
		drawgui();

		// update the highlighted length
		/*highlight_lineamount = (highlight_firstline>highlight_lastline)?
				highlight_firstline - highlight_lastline +1
				: highlight_lastline - highlight_firstline +1;
				*/
	}
	highlight_firstline = -1;
	highlight_lastline = -1;

	return;
}
