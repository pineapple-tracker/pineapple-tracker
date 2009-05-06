/* vi:set ts=4 sts=4 sw=4: */
#include "pineapple.h"

//volatile u8 callbackwait;
//u8 callbacktime = 180;
//u8 interruptwait = 0;

u8 trackwait;
u8 trackpos;
u8 songpos;

u8 playsong;
u8 playtrack;

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
	u8	tracknum;
	s8	transp;
	u8	tracknote;
	u8	lastinstr;
	u8	instrnum;
	u8	instrptr;
	u8	instrwait;
	u8	instrnote;
	s8	benddelta;
	s16	bend;
	s8	volumedelta;
	s16	dutydelta;
	u8	vdepth;
	u8	vrate;
	u8	vpos;
	s16	inertia;
	u16	slur;
} channel[4];

void runcmd(u8 ch, u8 cmd, u8 param){
	switch(cmd){
		case 'd':
			osc[ch].duty = param << 8;
			break;
		case 'f':
			channel[ch].volumedelta = param;
			break;
		case 'i':
			channel[ch].inertia = param << 1;
			break;
		case 's':
			channel[ch].benddelta = param;
			break;
		case 'm':
			channel[ch].dutydelta = param << 6;
			break;
		case 't':
			channel[ch].instrwait = param;
			break;
		case 'v':
			osc[ch].volume = param;
			break;
		case 'w':
			osc[ch].waveform = param;
			break;
		case '@':
			channel[ch].instrptr = param;
			break;
		case '=':
			channel[ch].instrnote = param;
			break;
		case '+':
			// 12 * 4 = C4;
			channel[ch].instrnote = param + channel[ch].tracknote - 12 * 4;
			break;
		case '~':
			if(channel[ch].vdepth != (param >> 4)){
				channel[ch].vpos = 0;
			}
			channel[ch].vdepth = param >> 4;
			channel[ch].vrate = param & 0xf;
			break;
		case '*':
			callbacktime = -param;
			break;
	}
}

void silence(void){
	for(u8 i = 0; i < 4; i++){
		osc[i].volume = 0;
	}
	playsong = 0;
	playtrack = 0;
}

void iedplonk(int note, int instr){
	channel[0].tracknote = note;
	channel[0].instrnum = instr;
	channel[0].instrptr = 0;
	channel[0].instrwait = 0;
	channel[0].bend = 0;
	channel[0].benddelta = 0;
	channel[0].volumedelta = 0;
	channel[0].dutydelta = 0;
	channel[0].vdepth = 0;
}

void startplaytrack(int t){
	channel[0].tracknum = t;
	channel[1].tracknum = 0;
	channel[2].tracknum = 0;
	channel[3].tracknum = 0;
	trackpos = 0;
	trackwait = 0;
	playtrack = 1;
	playsong = 0;
}

void startplaysong(int p){
	songpos = p;
	trackpos = 0;
	trackwait = 0;
	playtrack = 0;
	playsong = 1;
}

void initchip(void){
	trackwait = 0;
	trackpos = 0;
	playsong = 0;
	playtrack = 0;

	osc[0].volume = 0;
	channel[0].instrnum = 0;
	osc[1].volume = 0;
	channel[1].instrnum = 0;
	osc[2].volume = 0;
	channel[2].instrnum = 0;
	osc[3].volume = 0;
	channel[3].instrnum = 0;
}

/*void playroutine(void){
}*/

void process_(struct trackline *tl, struct oscillator *o){
}

u8 interrupthandler(void){
	u8 hai = 0;

	//playroutine();

	for(u8 i=0; i<4; i++){
		s8 amplitude;

		switch(osc[i].waveform){
			case WF_SAW:
				break;
			default:
				break;
		}
		osc[i].phase += osc[i].freq;
	}

	return hai;
}
