#include "pineapple.h"
#include "gui.h"
#include "hvl_replay.h"

/* move cursor left one column */
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

/* move cursor right one column */
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

/* move cursor up 1 line */
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

/* move cursor down 1 line */
void act_mvdown(void){
	switch(currtab){
		case 0:
			if(songy < tune->ht_PositionNr - 1){
				songy++;
			}else{
				songy = tune->ht_PositionNr - 1;
			}
			break;
		case 1:
			if(tracky < tune->ht_TrackNr - 1){
				tracky++;
			}else{
				tracky = tune->ht_TrackNr - 1;
			}
			break;
		case 2:
			if(instry < tune->ht_Instruments[currinstr].ins_PList.pls_Length - 1){
				instry++;
			}else{
				instry = tune->ht_Instruments[currinstr].ins_PList.pls_Length - 1;
			}
			break;
	}
}
