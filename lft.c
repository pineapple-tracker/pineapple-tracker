//---------------------------------------------------------------------------\\
// lft.c
// lft's musicchip song format
//---------------------------------------------------------------------------//

#include <stdio.h>
#include <string.h>

#include "pineapple.h"
#include "gui.h"
#include "filetypes.h"
#include "hvl_replay.h"
#include "lft.h"

volatile u8 callbackwait;
u8 callbacktime = 180;

u8 trackwait;
u8 trackpos;
u8 interruptwait = 0;

/*const u16 freqtable[] = {
	0x010b, 0x011b, 0x012c, 0x013e, 0x0151, 0x0165, 0x017a, 0x0191, 0x01a9,
	0x01c2, 0x01dd, 0x01f9, 0x0217, 0x0237, 0x0259, 0x027d, 0x02a3, 0x02cb,
	0x02f5, 0x0322, 0x0352, 0x0385, 0x03ba, 0x03f3, 0x042f, 0x046f, 0x04b2,
	0x04fa, 0x0546, 0x0596, 0x05eb, 0x0645, 0x06a5, 0x070a, 0x0775, 0x07e6,
	0x085f, 0x08de, 0x0965, 0x09f4, 0x0a8c, 0x0b2c, 0x0bd6, 0x0c8b, 0x0d4a,
	0x0e14, 0x0eea, 0x0fcd, 0x10be, 0x11bd, 0x12cb, 0x13e9, 0x1518, 0x1659,
	0x17ad, 0x1916, 0x1a94, 0x1c28, 0x1dd5, 0x1f9b, 0x217c, 0x237a, 0x2596,
	0x27d3, 0x2a31, 0x2cb3, 0x2f5b, 0x322c, 0x3528, 0x3851, 0x3bab, 0x3f37,
	0x42f9, 0x46f5, 0x4b2d, 0x4fa6, 0x5462, 0x5967, 0x5eb7, 0x6459, 0x6a51,
	0x70a3, 0x7756, 0x7e6f
};*/

// for 16kHz
/*const u16 freqtable[] = {
	0x0085, 0x008d, 0x0096, 0x009f, 0x00a8, 0x00b2, 0x00bd, 0x00c8, 0x00d4,
	0x00e1, 0x00ee, 0x00fc, 0x010b, 0x011b, 0x012c, 0x013e, 0x0151, 0x0165,
	0x017a, 0x0191, 0x01a9, 0x01c2, 0x01dd, 0x01f9, 0x0217, 0x0237, 0x0259,
	0x027d, 0x02a3, 0x02cb, 0x02f5, 0x0322, 0x0352, 0x0385, 0x03ba, 0x03f3,
	0x042f, 0x046f, 0x04b2, 0x04fa, 0x0546, 0x0596, 0x05eb, 0x0645, 0x06a5,
	0x070a, 0x0775, 0x07e6, 0x085f, 0x08de, 0x0965, 0x09f4, 0x0a8c, 0x0b2c,
	0x0bd6, 0x0c8b, 0x0d4a, 0x0e14, 0x0eea, 0x0fcd, 0x10be, 0x11bd, 0x12cb,
	0x13e9, 0x1518, 0x1659, 0x17ad, 0x1916, 0x1a94, 0x1c28, 0x1dd5, 0x1f9b,
	0x217c, 0x237a, 0x2596, 0x27d3, 0x2a31, 0x2cb3, 0x2f5b, 0x322c, 0x3528,
	0x3851, 0x3bab, 0x3f37
};*/

// for 48kHz
const u16 freqtable[] = {
	0x002c, 0x002f, 0x0032, 0x0035, 0x0038, 0x003b, 0x003f, 0x0042, 0x0046,
	0x004b, 0x004f, 0x0054, 0x0059, 0x005e, 0x0064, 0x006a, 0x0070, 0x0077,
	0x007e, 0x0085, 0x008d, 0x0096, 0x009f, 0x00a8, 0x00b2, 0x00bd, 0x00c8,
	0x00d4, 0x00e1, 0x00ee, 0x00fc, 0x010b, 0x011b, 0x012c, 0x013e, 0x0151, 
	0x0165, 0x017a, 0x0190, 0x01a8, 0x01c2, 0x01dc, 0x01f9, 0x0217, 0x0237,
	0x0258, 0x027c, 0x02a2, 0x02ca, 0x02f4, 0x0321, 0x0351, 0x0384, 0x03b9,
	0x03f2, 0x042e, 0x046e, 0x04b1, 0x04f8, 0x0544, 0x0594, 0x05e9, 0x0643,
	0x06a3, 0x0708, 0x0773, 0x07e4, 0x085c, 0x08dc, 0x0962, 0x09f1, 0x0a89,
	0x0b29, 0x0bd3, 0x0c87, 0x0d46, 0x0e10, 0x0ee6, 0x0fc9, 0x10b9, 0x11b8,
	0x12c5, 0x13e3, 0x1512
};

const s8 sinetable[] = {
	0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126,
	127, 126, 125, 122, 117, 112, 106, 98, 90, 81, 71, 60, 49, 37, 25, 12,
	0, -12, -25, -37, -49, -60, -71, -81, -90, -98, -106, -112, -117, -122,
	-125, -126, -127, -126, -125, -122, -117, -112, -106, -98, -90, -81,
	-71, -60, -49, -37, -25, -12
};

struct channel {
	u8	tnum;
	s8	transp;
	u8	tnote;
	u8	lastinstr;
	u8	inum;
	u8	iptr;
	u8	iwait;
	u8	inote;
	s8	bendd;
	s16	bend;
	s8	volumed;
	s16	dutyd;
	u8	vdepth;
	u8	vrate;
	u8	vpos;
	s16	inertia;
	u16	slur;
} channel[4];

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void readtrack(int,int,trackline) >                                   .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void readtrack(int num, int pos, struct trackline *tl){
	tl->note = tune->trk[num].line[pos].note;
	tl->instr = tune->trk[num].line[pos].instr;
	tl->cmd[0] = tune->trk[num].line[pos].cmd[0];
	tl->cmd[1] = tune->trk[num].line[pos].cmd[1];
	tl->param[0] = tune->trk[num].line[pos].param[0];
	tl->param[1] = tune->trk[num].line[pos].param[1];
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void runcmd(u8,u8,u8) >                                               .|
///  Executes a song command.                                                .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void runcmd(u8 ch, u8 cmd, u8 param){
	switch(cmd){
		case 0:
			channel[ch].inum = 0;
			break;
		case 'd':
			osc[ch].duty = param << 8;
			break;
		case 'f':
			channel[ch].volumed = param;
			break;
		case 'i':
			channel[ch].inertia = param << 1;
			break;
		case '@':
			channel[ch].iptr = param;
			break;
		case 's':
			channel[ch].bendd = param;
			break;
		case 'm':
			channel[ch].dutyd = param << 6;
			break;
		case 't':
			channel[ch].iwait = (param*6);
			break;
		case 'v':
			osc[ch].volume = param;
			break;
		case 'w':
			osc[ch].waveform = param;
			break;
		case '+':
			channel[ch].inote = param + channel[ch].tnote - 12 * 4;
			break;
		case '=':
			channel[ch].inote = param;
			break;
		case '~':
			if(channel[ch].vdepth != (param >> 4)){
				channel[ch].vpos = 0;
			}
			channel[ch].vdepth = param >> 4;
			channel[ch].vrate = (param/2) & 0xf;
			//channel[ch].vrate = param & 0xf;
			break;
		case '*':
			callbacktime = -param;
			break;
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void silence() >                                                      .|
///  Stops all sound.                                                        .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void silence(void){
	for(u8 i = 0; i < 4; i++){
		osc[i].volume = 0;
	}
	playsong = 0;
	playtrack = 0;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void lft_iedplonk(int,int) >                                              .|
///  Plays a note.                                                           .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void lft_iedplonk(int note, int instr, pineapple_tune *t){
	channel[0].tnote = note;
	channel[0].inum = instr;
	channel[0].iptr = 0;
	channel[0].iwait = 0;
	channel[0].bend = 0;
	channel[0].bendd = 0;
	channel[0].volumed = 0;
	channel[0].dutyd = 0;
	channel[0].vdepth = 0;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void startplaytrack(int) >                                            .|
///  Plays a track.                                                          .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void startplaytrack(int t){
	channel[0].tnum = t;
	channel[1].tnum = 0;
	channel[2].tnum = 0;
	channel[3].tnum = 0;
	trackpos = 0;
	trackwait = 0;
	playtrack = 1;
	playsong = 0;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void startplaysong(int) >                                             .|
///  Plays the song from a certain position.                                 .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void startplaysong(int p){
	tune->songpos = p;
	trackpos = 0;
	trackwait = 0;
	playtrack = 0;
	playsong = 1;
}

void playroutine(){			// called at 50 Hz
	u8 ch;

	if(playtrack || playsong){
		if(trackwait){
			trackwait--;
		}else{
			trackwait = 12;
			//trackwait = 4;

			if(!trackpos){
				if(playsong){
					if(tune->songpos >= tune->songlen){
						playsong = 0;
					}else{
						for(ch = 0; ch < 4; ch++){
							u8 tmp[2];

							readsong(tune->songpos, ch, tmp);
							channel[ch].tnum = tmp[0];
							channel[ch].transp = tmp[1];
						}
						tune->songpos++;
					}
				}
			}

			if(playtrack || playsong){
				for(ch = 0; ch < 4; ch++){
					if(channel[ch].tnum){
						struct trackline tl;
						u8 instr = 0;

						readtrack(channel[ch].tnum, trackpos, &tl);
						if(tl.note){
							channel[ch].tnote = tl.note + channel[ch].transp;
							instr = channel[ch].lastinstr;
						}
						if(tl.instr){
							instr = tl.instr;
						}
						if(instr){
							channel[ch].lastinstr = instr;
							channel[ch].inum = instr;
							channel[ch].iptr = 0;
							channel[ch].iwait = 0;
							channel[ch].bend = 0;
							channel[ch].bendd = 0;
							channel[ch].volumed = 0;
							channel[ch].dutyd = 0;
							channel[ch].vdepth = 0;
						}
						if(tl.cmd[0])
							runcmd(ch, tl.cmd[0], tl.param[0]);
						if(tl.cmd[1])
							runcmd(ch, tl.cmd[1], tl.param[1]);
					}
				}

				trackpos++;
				trackpos &= 31;
			}
		}
	}

	for(ch = 0; ch < 4; ch++){
		s16 vol;
		u16 duty;
		u16 slur;

		// i dunno if that last condition is correct...........................
		while((channel[ch].inum && !channel[ch].iwait) || channel[0].iptr == 0){
			u8 il[2];

			readinstr(channel[ch].inum, channel[ch].iptr, il);
			channel[ch].iptr++;

			runcmd(ch, il[0], il[1]);
		}
		if(channel[ch].iwait) channel[ch].iwait--;

		if(channel[ch].inertia){
			s16 diff;

			slur = channel[ch].slur;
			diff = freqtable[channel[ch].inote] - slur;
			//diff >>= channel[ch].inertia;
			if(diff > 0){
				if(diff > channel[ch].inertia) diff = channel[ch].inertia;
			}else if(diff < 0){
				if(diff < -channel[ch].inertia) diff = -channel[ch].inertia;
			}
			slur += diff;
			channel[ch].slur = slur;
		}else{
			slur = freqtable[channel[ch].inote];
		}
		osc[ch].freq =
			slur +
			channel[ch].bend +
			((channel[ch].vdepth * sinetable[channel[ch].vpos & 63]) >> 2);
		channel[ch].bend += channel[ch].bendd;
		vol = osc[ch].volume + channel[ch].volumed;
		if(vol < 0) vol = 0;
		if(vol > 255) vol = 255;
		osc[ch].volume = vol;

		duty = osc[ch].duty + channel[ch].dutyd;
		if(duty > 0xe000) duty = 0x2000;
		if(duty < 0x2000) duty = 0xe000;
		osc[ch].duty = duty;

		channel[ch].vpos += channel[ch].vrate;
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void initchip() >                                                     .|
///  Initialize sound engine.                                                .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void initchip(){
	trackwait = 0;
	trackpos = 0;
	playsong = 0;
	playtrack = 0;

	osc[0].volume = 0;
	channel[0].inum = 0;
	osc[1].volume = 0;
	channel[1].inum = 0;
	osc[2].volume = 0;
	channel[2].inum = 0;
	osc[3].volume = 0;
	channel[3].inum = 0;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void interrupthandler(int,int) >                                      .|
///  Called by sdl_callbackbuffer. Computes a frame of audio.                .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
u8 interrupthandler()        // called at 9000 Hz
{
	u8 i;
	u8 j = 0;
	s16 acc;
	static u32 noiseseed = 1;
	u8 newbit = 0;

	if(noiseseed & 0x80000000L) newbit ^= 1;
	if(noiseseed & 0x01000000L) newbit ^= 1;
	if(noiseseed & 0x00000040L) newbit ^= 1;
	if(noiseseed & 0x00000200L) newbit ^= 1;
	noiseseed = (noiseseed << 1) | newbit;

	if(callbackwait){
		callbackwait--;
	}else{
		playroutine();
		callbackwait = callbacktime - 1;
	}

	acc = 0;
	for(i = 0; i < 4; i++){
		s8 value; // [-32,31]

		switch(osc[i].waveform){
			case WF_TRI:
				if(osc[i].phase < 0x8000){
					value = -32 + (osc[i].phase >> 9);
				}else{
					value = 31 - ((osc[i].phase - 0x8000) >> 9);
				}
				break;
			case WF_SAW:
				value = -32 + (osc[i].phase >> 10);
				break;
			case WF_PUL:
				value = (osc[i].phase > osc[i].duty)? -32 : 31;
				break;
			case WF_NOI:
				value = (noiseseed & 63) - 32;
				break;
			case WF_SINE:
				value = sinetable[j];
				if(j >= sizeof(sinetable)-1) j = 0;
				else j++;
			default:
				value = 0;
				break;
		}

		osc[i].phase += osc[i].freq;

		acc += value * osc[i].volume; // rhs = [-8160,7905]
	}

	// acc [-32640,31620]
	return 128 + (acc >> 8);	// [1,251]
}


//--------------------------------------------------------------------------\\
// File operations
//--------------------------------------------------------------------------//

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void lft_savefile(char *fname) >                                      .|
///  Save the currently loaded lft tune.                                     .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void lft_savefile(char *fname){
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
	fprintf(f, "%s\n", filename);
	fprintf(f, "\n");
	fprintf(f, "#%s\n", comment);
	fprintf(f, "\n");
	fprintf(f, "tempo: %d\n", callbacktime);
	for(i = 0; i < tune->songlen; i++){
		fprintf(f, "songline %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i,
			tune->sng[i].track[0],
			tune->sng[i].transp[0],
			tune->sng[i].track[1],
			tune->sng[i].transp[1],
			tune->sng[i].track[2],
			tune->sng[i].transp[2],
			tune->sng[i].track[3],
			tune->sng[i].transp[3]);
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++){
		for(j = 0; j < tune->tracklen; j++){
			struct trackline *tl = &tune->trk[i].line[j];

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

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < pineapple_tune *lft_loadfile(char *) >                                .|
///  Takes a filename. Returns a pineapple_tune struct. NULL on failure      .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
pineapple_tune *lft_loadfile(char *fname){
	pineapple_tune *t;
	FILE *f;
	char header[4];
	char buf[1024];
	int cmd[3];
	int i1, i2, trk[4], transp[4], param[3], note, instr;
	int i;
	t = (pineapple_tune*) malloc(sizeof(pineapple_tune));
	if(!t) {
		fprintf(stderr, "couldn't malloc pineapple_tune *tune!\n");
		return NULL;
	}

	snprintf(filename, sizeof(filename), "%s", fname);
	//snprintf(t->filename, sizeof(filename), "%s", fname);

	f = fopen(fname, "r");
	if(!f){
		return NULL;
	}

	//check if its a musicchip file so we can return from this function and try to load the next type of file
	fseek(f, 0, SEEK_SET);
	fread(&header, 1, 4, f);
	if(strncmp(header, "musi", sizeof(header)) == 0){
		fprintf(stderr, "loading .lft file\n");
	}else {
		fprintf(stderr, "not a .lft file!\n");
		fprintf(stderr, "%s\n", header);
		return NULL;
	}
	rewind(f);

	t->type = LFT;
	t->songlen = 1;
	t->tracklen = TRACKLEN;
	while(!feof(f) && fgets(buf, sizeof(buf), f)){
		if(1 == sscanf(buf, "#%1024c", &comment)){
		}
		else if(1 == sscanf(buf, "tempo: %hhd", &callbacktime)){
			t->callbacktime = (u8)callbacktime;
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
				t->sng[i1].track[i] = trk[i];
				t->sng[i1].transp[i] = transp[i];
			}
			if(t->songlen <= i1) t->songlen = i1 + 1;
		}else if(8 == sscanf(buf, "trackline %x %x %x %x %x %x %x %x",
			&i1,
			&i2,
			&note,
			&instr,
			&cmd[0],
			&param[0],
			&cmd[1],
			&param[1])){

			t->trk[i1].line[i2].note = note;
			t->trk[i1].line[i2].instr = instr;
			for(i = 0; i < 2; i++){
				t->trk[i1].line[i2].cmd[i] = cmd[i];
				t->trk[i1].line[i2].param[i] = param[i];
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

	t->iedplonk = lft_iedplonk;

	fclose(f);
	return t;
}

void lft_saveinstrument(char *fname){
	FILE *f;
	int i;

	f = fopen(fname, "w");
	if(!f){
		fprintf(stderr, "save error!\n");
		return;
	}
	
	fprintf(f, "pineapple tune instrument\n");
	fprintf(f, "version alphamega\n");
	fprintf(f, "\n");
	fprintf(f, "%s\n", filename);
	fprintf(f, "\n");
	for(i = 0; i < instrument[currinstr].length; i++){
			fprintf(f, "instrumentline %02x %02x %02x\n",
				i,
				instrument[currinstr].line[i].cmd,
				instrument[currinstr].line[i].param);
	}
	fclose(f);
	return;
}

int lft_loadinstrument(char *fname){
	FILE *f;
	char buf[1024];
	int i, cmd[3], param[3];
	int fr;

	f = fopen(fname, "r");
	if(!f){
		return -1;
	}
	
	fr = nextfreeinstr();

	while(!feof(f) && fgets(buf, sizeof(buf), f)){
		if(3 == sscanf(buf, "instrumentline %x %x %x",
			&i,
			&cmd[0],
			&param[0])){

			instrument[fr].line[i].cmd = cmd[0];
			instrument[fr].line[i].param = param[0];
			if(instrument[fr].length <= i) instrument[fr].length = i + 1;
		}
	}

	fclose(f);
	return 0;
}
