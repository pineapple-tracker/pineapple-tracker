/* vi:set ts=8 sts=8 sw=8 noexpandtab: */
#include "pineapple.h"
#include "filetypes.h"
#include "gui.h"

#include <string.h>

static u8 _oldfxparam = 0;

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
			if(songy < tune->songlen - 1){
				songy++;
			}else{
				songy = tune->songlen - 1;
			}
			break;
		case 1:
			if(tracky < tune->tracklen - 1){
				tracky++;
			}else{
				tracky = tune->tracklen - 1;
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

/* move cursor up 8 lines */
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

/* move cursor down 8 lines */
void act_bigmvdown(void){
	switch(currtab){
		case 0:
			if(songy < tune->songlen - 8){
				songy += 8;
			}else{
				songy = tune->songlen - 1;
			}
			break;
		case 1:
			if(tracky < tune->tracklen - 8){
				tracky += 8;
			}else{
				tracky = tune->tracklen - 1;
			}
			break;
		case 2:
			if(instry < instrument[currinstr].length - 8) instry += 8;
			break;
	}
}

void act_mvbottom(void){
	switch(currtab){
		case 0:
			songy = tune->songlen - 1;
			break;
		case 1:
			tracky = tune->tracklen - 1;
			break;
		case 2:
			instry = instrument[currinstr].length - 1;
			break;
	}
}

void act_mvtop(void){
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

void act_viewtrackinc(void){
	if(currtrack < 0xff){
		currtrack++;
	}else if(currtrack == 0xff){
		currtrack = 1;
	}
	if(playtrack){
		startplaytrack(currtrack);
	}
}

void act_viewtrackdec(void){
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
			tune->sng[songy].track[songx/4] -= 240;
		}else{
			tune->sng[songy].track[songx/4] += 16;
		}
	}else{
		if( (tune->sng[songy].track[songx/4] % 16) == 15){
			tune->sng[songy].track[songx/4] -= 15;
		}else{
			tune->sng[songy].track[songx/4]++;
		}
	}
	saved = 0;
}

void act_trackdec(void){
	if( (songx%2)==0 ){
		if(songx <= 15 && songx >= 0){
			tune->sng[songy].track[songx/4] += 240;
		}else{
			tune->sng[songy].track[songx/4] -= 16;
		}
	}else{
		if( (tune->sng[songy].track[songx/4] % 16) == 0){
			tune->sng[songy].track[songx/4] += 15;
		}else{
			tune->sng[songy].track[songx/4]--;
		}
	}
	saved = 0;
}

void act_transpinc(void){
	if( (songx%2)==0 ){
		if(songx >= 240 && songx < 256){
			tune->sng[songy].transp[songx/4] -= 240;
		}else{
			tune->sng[songy].transp[songx/4] += 16;
		}
	}else{
		if( (tune->sng[songy].transp[songx/4] % 16) == 15){
			tune->sng[songy].transp[songx/4] -= 15;
		}else{
			tune->sng[songy].transp[songx/4]++;
		}
	}
	saved = 0;
}

void act_transpdec(void){
	if( (songx%2)==0 ){
		if(songx <= 15 && songx >= 0){
			tune->sng[songy].transp[songx/4] += 240;
		}else{
			tune->sng[songy].transp[songx/4] -= 16;
		}
	}else{
		if( (tune->sng[songy].transp[songx/4] % 16) == 0){
			tune->sng[songy].transp[songx/4] += 15;
		}else{
			tune->sng[songy].transp[songx/4]--;
		}
	}
	saved = 0;
}

void act_undo(void){
}

void act_noteinc(void){
	if(currtab==1){
		// if current note < H7
		if( tune->trk[currtrack].line[tracky].note < 96 ){
			tune->trk[currtrack].line[tracky].note++;
		}else{
			tune->trk[currtrack].line[tracky].note = 0;
		}
	}else if(currtab==2){
		if( instrument[currinstr].line[instry].param < 96 ){
			instrument[currinstr].line[instry].param++;
		}else{
			instrument[currinstr].line[instry].param = 0;
		}
	}
	saved = 0;
}

void act_notedec(void){
	if(currtab==1){
		if( tune->trk[currtrack].line[tracky].note > 0 ){
			tune->trk[currtrack].line[tracky].note--;
		}else{
			tune->trk[currtrack].line[tracky].note = 96;
		}
	}else if(currtab==2){
		if( instrument[currinstr].line[instry].param > 0 ){
			instrument[currinstr].line[instry].param--;
		}else{
			instrument[currinstr].line[instry].param = 96;
		}
	}
	saved = 0;
}

void act_octaveinc(void){
	if(currtab==1){
		if( tune->trk[currtrack].line[tracky].note+12 <= 96 ){
			tune->trk[currtrack].line[tracky].note+=12;
		}else{
			tune->trk[currtrack].line[tracky].note %= 12;
		}
	}else if(currtab==2){ if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
			if( instrument[currinstr].line[instry].param+12 <= 96 ){
				instrument[currinstr].line[instry].param+=12;
			}else{
				instrument[currinstr].line[instry].param %= 12;
			}
		}
	}
	saved = 0;
}

void act_octavedec(void){
	if(currtab==1){
		if( tune->trk[currtrack].line[tracky].note-12 > 0 ){
			tune->trk[currtrack].line[tracky].note-=12;
		}else{
			tune->trk[currtrack].line[tracky].note = 84 + tune->trk[currtrack].line[tracky].note;
		}
	}else if(currtab==2){
		if(instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '='){
			if( instrument[currinstr].line[instry].param-12 > 0 ){
				instrument[currinstr].line[instry].param-=12;
			}else{
				instrument[currinstr].line[instry].param = 84 + instrument[currinstr].line[instry].param;
			}
		}
	}
	saved = 0;
}

void act_instrinc(void){
	switch(trackx){
		case 2:
			SETHI(tune->trk[currtrack].line[tracky].instr,
					hexinc(tune->trk[currtrack].line[tracky].instr >> 4) );
			break;
		case 3:
			SETLO(tune->trk[currtrack].line[tracky].instr,
					hexinc(tune->trk[currtrack].line[tracky].instr & 0x0f) );
			break;
	}
	saved = 0;
}

void act_instrdec(void){
	switch(trackx){
		case 2:
			SETHI(tune->trk[currtrack].line[tracky].instr,
					hexdec(tune->trk[currtrack].line[tracky].instr >> 4) );
			break;
		case 3:
			SETLO(tune->trk[currtrack].line[tracky].instr,
					hexdec(tune->trk[currtrack].line[tracky].instr & 0x0f) );
			break;
	}
	saved = 0;
}

void act_fxinc(void){
	if(currtab==1){
		currcmd = tune->trk[currtrack].line[tracky].cmd[trackx % 2];
		// there must be a better way to do this...
		if((unsigned long)currcmd == (unsigned long)NULL){
			tune->trk[currtrack].line[tracky].cmd[trackx % 2] = validcmds[0];
		}else{
			for(int z = 0; z < strlen(validcmds); z++){
				if(currcmd == validcmds[z]){
					if(z == (strlen(validcmds)-1)){
						tune->trk[currtrack].line[tracky].cmd[trackx % 2] = (unsigned long)NULL;
					}else{
						tune->trk[currtrack].line[tracky].cmd[trackx % 2] = validcmds[z+1];
					}
					continue;
				}
			}
		}
	}else if(currtab==2){
		currcmd = instrument[currinstr].line[instry].cmd;
		for(int z = 0; z < strlen(validcmds); z++){
			if(currcmd == validcmds[z]){
				if(z == (strlen(validcmds)-1)){
					instrument[currinstr].line[instry].cmd = validcmds[0];
				}else{
					instrument[currinstr].line[instry].cmd = validcmds[z+1];
					if(_oldfxparam)
						instrument[currinstr].line[instry].param = _oldfxparam;
				}

				// when switching to the note command, change to param if it's
				// higher than H7
				if((instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '=')
				//&& _oldfxparam
				&& instrument[currinstr].line[instry].param>96){
					// save current param
					_oldfxparam = instrument[currinstr].line[instry].param;
					instrument[currinstr].line[instry].param = 96; //H7
				}else if(_oldfxparam){
					instrument[currinstr].line[instry].param = _oldfxparam;
					_oldfxparam = 0;
				}

				continue;
			}
		}
	}
	saved = 0;
}

void act_fxdec(void){
	if(currtab==1){
		currcmd = tune->trk[currtrack].line[tracky].cmd[trackx % 2];
		if((unsigned long)currcmd == (unsigned long)NULL){
			tune->trk[currtrack].line[tracky].cmd[trackx % 2] = validcmds[strlen(validcmds)-1];
		}else{
			for(int z = 0; z < strlen(validcmds); z++){
				if(currcmd == validcmds[z]){
					if(z==0){
						tune->trk[currtrack].line[tracky].cmd[trackx % 2] = (unsigned long)NULL;
					}else{
						tune->trk[currtrack].line[tracky].cmd[trackx % 2] = validcmds[z-1];
					}
					continue;
				}
			}
		}
	}else if(currtab==2){
		currcmd = instrument[currinstr].line[instry].cmd;
		for(int z = 0; z < strlen(validcmds); z++){
			if(currcmd == validcmds[z]){
				if(z==0){
					instrument[currinstr].line[instry].cmd = validcmds[strlen(validcmds)-1];
				}else{
					instrument[currinstr].line[instry].cmd = validcmds[z-1];
					if(_oldfxparam)
						instrument[currinstr].line[instry].param = _oldfxparam;
				}

				// when switching to the note command, change to param if it's
				// higher than H7
				if((instrument[currinstr].line[instry].cmd == '+' || instrument[currinstr].line[instry].cmd == '=')
				//&& _oldfxparam
				&& instrument[currinstr].line[instry].param>96){
					// save current param
					_oldfxparam = instrument[currinstr].line[instry].param;
					instrument[currinstr].line[instry].param = 96; //H7
				}else if(_oldfxparam){
					instrument[currinstr].line[instry].param = _oldfxparam;
					_oldfxparam = 0;
				}

				continue;
			}
		}
	}
	saved = 0;
}

void act_paraminc(void){
	if(currtab==1){
		if((trackx==5 && tune->trk[currtrack].line[tracky].cmd[0])
				|| (trackx==8 && tune->trk[currtrack].line[tracky].cmd[1])){
			SETHI(tune->trk[currtrack].line[tracky].param[(trackx - 1) % 2],
					hexinc(tune->trk[currtrack].line[tracky].param[(trackx - 1) % 2] >> 4) );
			return;
		}else if((trackx==6 && tune->trk[currtrack].line[tracky].cmd[0])
				|| (trackx==9 && tune->trk[currtrack].line[tracky].cmd[1])){
			SETLO(tune->trk[currtrack].line[tracky].param[trackx % 2],
					hexinc(tune->trk[currtrack].line[tracky].param[trackx % 2] & 0x0f) );
			return;
		}
	}else if(currtab == 2){
		if(instrx == 1){
			SETHI(instrument[currinstr].line[instry].param,
					hexinc(instrument[currinstr].line[instry].param >> 4) );
			return;
		}else if(instrx == 2){
			SETLO(instrument[currinstr].line[instry].param,
					hexinc(instrument[currinstr].line[instry].param & 0x0f) );
			return;
		}
	}
	saved = 0;
}

void act_paramdec(void){
	if(currtab==1){
		if((trackx==5 && tune->trk[currtrack].line[tracky].cmd[0])
				|| (trackx==8 && tune->trk[currtrack].line[tracky].cmd[1])){
			SETHI(tune->trk[currtrack].line[tracky].param[(trackx-1) % 2],
					hexdec(tune->trk[currtrack].line[tracky].param[(trackx-1) % 2] >> 4) );
			return;
		}else if((trackx==6 && tune->trk[currtrack].line[tracky].cmd[0])
				|| (trackx==9 && tune->trk[currtrack].line[tracky].cmd[1])){
			SETLO(tune->trk[currtrack].line[tracky].param[trackx % 2],
					hexdec(tune->trk[currtrack].line[tracky].param[trackx % 2] & 0x0f) );
			return;
		}
	}else if(currtab == 2){
		if(instrx == 1){
			SETHI(instrument[currinstr].line[instry].param,
					hexdec(instrument[currinstr].line[instry].param >> 4) );
			return;
		}else if(instrx == 2){
			SETLO(instrument[currinstr].line[instry].param,
					hexdec(instrument[currinstr].line[instry].param & 0x0f) );
			return;
		}
	}
	saved = 0;
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
		if(tune->songlen < 256){
			memmove(&tune->sng[songy + 2], &tune->sng[songy + 1], sizeof(struct songline) * (tune->songlen - songy - 1));
			songy++;
			tune->songlen++;
			memset(&tune->sng[songy], 0, sizeof(struct songline));
		}
	}
	saved = 0;
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
		if(tune->songlen > 1){
			memmove(&tune->sng[songy + 0], &tune->sng[songy + 1], sizeof(struct songline) * (tune->songlen - songy - 1));
			tune->songlen--;
			if(songy >= tune->songlen) songy = tune->songlen - 1;
		}
	}
	saved = 0;
}

void act_clronething(void){
	if(currtab == 0){
		if( (songx%4) < 2){
			if( (songx%2)==0 ){
				tune->sng[songy].track[songx/4] = (tune->sng[songy].track[songx/4] - tune->sng[songy].track[songx/4]) + tune->sng[songy].track[songx/4]%16;
			}else{
				tune->sng[songy].track[songx/4] -= tune->sng[songy].track[songx/4]%16;
			}
		}else{
			if( (songx%2)==0 ){
				tune->sng[songy].transp[songx/4] = (tune->sng[songy].transp[songx/4] - tune->sng[songy].transp[songx/4]) + tune->sng[songy].transp[songx/4]%16;
			}else{
				tune->sng[songy].transp[songx/4] -= tune->sng[songy].transp[songx/4]%16;
			}
		}
		//memcpy(&tclip, &tune->sng[songy], sizeof(struct songline));
	}else if(currtab == 1){
		switch(trackx){
			case 0:
				memcpy(&tclip, &tune->trk[currtrack].line[tracky], sizeof(struct trackline));
				tune->trk[currtrack].line[tracky].note = 0;
				tune->trk[currtrack].line[tracky].instr = 0;
				//memmove
				break;
			case 2:
				memcpy(&tclip, &tune->trk[currtrack].line[tracky].instr, sizeof(struct trackline));
				SETHI(tune->trk[currtrack].line[tracky].instr, 0);
				break;
			case 3:
				memcpy(&tclip, &tune->trk[currtrack].line[tracky].instr, sizeof(struct trackline));
				SETLO(tune->trk[currtrack].line[tracky].instr, 0);
				break;
			case 4:
				tune->trk[currtrack].line[tracky].cmd[0] = 0;
				break;
			case 5:
				SETHI(tune->trk[currtrack].line[tracky].param[0],0);
				break;
			case 6:
				SETLO(tune->trk[currtrack].line[tracky].param[0],0);
				break;
			case 7:
				tune->trk[currtrack].line[tracky].cmd[1] = 0;
				break;
			case 8:
				SETHI(tune->trk[currtrack].line[tracky].param[1],0);
				break;
			case 9:
				SETLO(tune->trk[currtrack].line[tracky].param[1],0);
				break;
			default:
				setdisplay("in ACT_CLRONETHING");
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
	saved = 0;
}

void act_clritall(void){
	if(currtab == 0){
		for(int ci = 0; ci < 4; ci++){
			tune->sng[songy].track[ci] = 0;
			tune->sng[songy].transp[ci] = 0;
		}
	}else if(currtab == 1){
		tune->trk[currtrack].line[tracky].note = 0;
		tune->trk[currtrack].line[tracky].instr = 0;
		SETHI(tune->trk[currtrack].line[tracky].instr, 0);
		SETLO(tune->trk[currtrack].line[tracky].instr, 0);
		tune->trk[currtrack].line[tracky].cmd[0] = 0;
		SETHI(tune->trk[currtrack].line[tracky].param[0],0);
		SETLO(tune->trk[currtrack].line[tracky].param[0],0);
		tune->trk[currtrack].line[tracky].cmd[1] = 0;
		SETHI(tune->trk[currtrack].line[tracky].param[1],0);
		SETLO(tune->trk[currtrack].line[tracky].param[1],0);
	}else if(currtab == 2){
		instrument[currinstr].line[instry].cmd = '0';
		instrument[currinstr].line[instry].param = 0;
	}
	saved = 0;
}

// clear line y in the song tab
void act_clrinsongtab(int y){
	for(int ci = 0; ci < 4; ci++){
		tune->sng[songy].track[ci] = 0;
		tune->sng[songy].transp[ci] = 0;
	}
	saved = 0;
}

void act_clrintracktab(int t, int y){
	tune->trk[t].line[y].note = 0;
	tune->trk[t].line[y].instr = 0;
	SETHI(tune->trk[t].line[y].instr, 0);
	SETLO(tune->trk[t].line[y].instr, 0);
	tune->trk[t].line[y].cmd[0] = 0;
	SETHI(tune->trk[t].line[y].param[0],0);
	SETLO(tune->trk[t].line[y].param[0],0);
	tune->trk[t].line[y].cmd[1] = 0;
	SETHI(tune->trk[t].line[y].param[1],0);
	SETLO(tune->trk[t].line[y].param[1],0);
	saved = 0;
}

void act_clrininstrtab(int instr, int y){
	instrument[instr].line[y].cmd = '0';
	instrument[instr].line[y].param = 0;
	saved = 0;
}
