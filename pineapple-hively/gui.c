#include <ncurses.h>
#include "hvl_replay.h"


void initgui()
{
	initscr();

	nonl();
	noecho();
	keypad(stdscr, FALSE);

	nodelay(stdscr, TRUE);
	
	//atexit(endwin());
}

void drawposed()
{
	int i, j;
	char buf[1024];
	move(0,0);
	for(i = 0; i<tune->ht_PositionNr; i++){
		snprintf(buf, sizeof(buf), "%02x", i);
		addstr(buf);
		addch(ACS_VLINE);
		for(j= 0; j<4; j++){
			snprintf(buf, sizeof(buf), "%02x:%02x", tune->ht_Positions[i].pos_Track[j], tune->ht_Positions[i].pos_Transpose[j]);
			addstr(buf);
			if(j !=3)
				addch(' ');
		}
		move(i + 1, 0);
	}
}

void drawtracked()
{
	int i, j;
	char buf[1024];
}

void drawgui()
{
	drawposed();
	//drawtracked();

}

void handleinput(){
	int c;
	if((c = getch()) != ERR){
		switch(c){
		case 'Q':
			erase();
			refresh();
			endwin();
			exit(0);
			break;
		default:
			break;
		}
	}
}

void guiloop()
{
	ESCDELAY=50;
	for(;;){
		drawgui();
		handleinput();
	}

}
