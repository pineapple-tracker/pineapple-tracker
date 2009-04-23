#include "stuff.h"
#include "gui.h"

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
			initsonglines();
			inittracks();
			initinstrs();
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
		setdisplay("not a tracker command!");
	return;
}

/* vi cmdline mode */
void cmdlinemode(void){
	u16 c;
	keypad(stdscr, TRUE);

	currmode = PM_CMDLINE;
	strncat(cmdstr, ":", 50);
	for(;;){
		drawgui();

		c = nextchar();
		switch(c){
			case KEY_ESCAPE:
				//cmdstr = "";
				currmode = PM_NORMAL;
				goto end;
			case ENTER:
				parsecmd(cmdstr);
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
				c = nextchar();
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

