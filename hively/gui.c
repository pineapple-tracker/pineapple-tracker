#include <ncurses.h>
#include "hvl_replay.h"

int currtrack = 1;

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
			snprintf(buf, sizeof(buf), "%02x  ", tune->ht_Tracks[currtrack][i].stp_Note);
		else
			snprintf(buf, sizeof(buf), "--- ");
		addstr(buf);

		if(tune->ht_Tracks[currtrack][i].stp_Instrument)
			snprintf(buf, sizeof(buf), "%02x  ", tune->ht_Tracks[currtrack][i].stp_Instrument);
		else
			snprintf(buf, sizeof(buf), "-- ");
		addstr(buf);
		move(i + 1, 35);
	}
}

void drawgui()
{
	drawposed();
	drawtracked();

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
		case 'J':
			if(currtrack > 1)
				currtrack--;
			break;
		case 'K':
			if(currtrack < 0xff)
				currtrack++;
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
