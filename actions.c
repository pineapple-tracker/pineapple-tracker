/* vi:set ts=4 sts=4 sw=4: */
#include "pineapple.h"
#include "gui.h"

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
	saved = 0;
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
	saved = 0;
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
	saved = 0;
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
	saved = 0;
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
	saved = 0;
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
	saved = 0;
}

void act_octaveinc(void){
	if(currtab==1){
		if( track[currtrack].line[tracky].note+12 <= 96 ){
			track[currtrack].line[tracky].note+=12;
		}else{
			track[currtrack].line[tracky].note %= 12;
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
		if( track[currtrack].line[tracky].note-12 > 0 ){
			track[currtrack].line[tracky].note-=12;
		}else{
			track[currtrack].line[tracky].note = 84 + track[currtrack].line[tracky].note;
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
			SETHI(track[currtrack].line[tracky].instr,
					hexinc(track[currtrack].line[tracky].instr >> 4) );
			break;
		case 3:
			SETLO(track[currtrack].line[tracky].instr,
					hexinc(track[currtrack].line[tracky].instr & 0x0f) );
			break;
	}
	saved = 0;
}

void act_instrdec(void){
	switch(trackx){
		case 2:
			SETHI(track[currtrack].line[tracky].instr,
					hexdec(track[currtrack].line[tracky].instr >> 4) );
			break;
		case 3:
			SETLO(track[currtrack].line[tracky].instr,
					hexdec(track[currtrack].line[tracky].instr & 0x0f) );
			break;
	}
	saved = 0;
}

void act_fxinc(void){
	if(currtab==1){
		currcmd = track[currtrack].line[tracky].cmd[trackx % 2];
		// there must be a better way to do this...
		if((unsigned long)currcmd == (unsigned long)NULL){
			track[currtrack].line[tracky].cmd[trackx % 2] = validcmds[0];
		}else{
			for(int z = 0; z < strlen(validcmds); z++){
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
		currcmd = track[currtrack].line[tracky].cmd[trackx % 2];
		if((unsigned long)currcmd == (unsigned long)NULL){
			track[currtrack].line[tracky].cmd[trackx % 2] = validcmds[strlen(validcmds)-1];
		}else{
			for(int z = 0; z < strlen(validcmds); z++){
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
		if((trackx==5 && track[currtrack].line[tracky].cmd[0])
				|| (trackx==8 && track[currtrack].line[tracky].cmd[1])){
			SETHI(track[currtrack].line[tracky].param[(trackx - 1) % 2],
					hexinc(track[currtrack].line[tracky].param[(trackx - 1) % 2] >> 4) );
			return;
		}else if((trackx==6 && track[currtrack].line[tracky].cmd[0])
				|| (trackx==9 && track[currtrack].line[tracky].cmd[1])){
			SETLO(track[currtrack].line[tracky].param[trackx % 2],
					hexinc(track[currtrack].line[tracky].param[trackx % 2] & 0x0f) );
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
		if((trackx==5 && track[currtrack].line[tracky].cmd[0])
				|| (trackx==8 && track[currtrack].line[tracky].cmd[1])){
			SETHI(track[currtrack].line[tracky].param[(trackx-1) % 2],
					hexdec(track[currtrack].line[tracky].param[(trackx-1) % 2] >> 4) );
			return;
		}else if((trackx==6 && track[currtrack].line[tracky].cmd[0])
				|| (trackx==9 && track[currtrack].line[tracky].cmd[1])){
			SETLO(track[currtrack].line[tracky].param[trackx % 2],
					hexdec(track[currtrack].line[tracky].param[trackx % 2] & 0x0f) );
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
		if(songlen < 256){
			memmove(&song[songy + 2], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
			songy++;
			songlen++;
			memset(&song[songy], 0, sizeof(struct songline));
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
		if(songlen > 1){
			memmove(&song[songy + 0], &song[songy + 1], sizeof(struct songline) * (songlen - songy - 1));
			songlen--;
			if(songy >= songlen) songy = songlen - 1;
		}
	}
	saved = 0;
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
	saved = 0;
}

