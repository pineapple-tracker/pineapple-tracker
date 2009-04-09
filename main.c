/* vi:set ts=4 sts=4 sw=4: */
#include <stdio.h>
#include <SDL/SDL.h>

#ifndef D_WINDOWS
#include <err.h>
#endif

#include "stuff.h"

/* These arrays don't appear to be used for anything...
consider removal?
*/

/*
const u16 instrptr[] = {
	0, 1
};

const u8 instrdata[] = {
	0, 0,
	'v', 255, 'm', 0x11, '+', 0, 'w', WF_PUL, 'f', 0xef, 't', 8, 'f', 0xfc, 0, 0, // lead
	'v', 255, 'w', WF_NOI, 't', 2, '+', 0, 'w', WF_SAW, 'f', 0xef, 't', 8, 'f', 0, 't', 12, '~', 0x37, 'f', 0xfc, 0, 0, // lead+vibr
	'v', 255, 'w', WF_NOI, 't', 2, 'm', 0x11, '+', 12, 'w', WF_PUL, 'f', 0xfc, 't', 2, '+', 7, 't', 2, '+', 3, 't', 2, '+', 0, 't', 2, 'j', 5, 0, 0, // arp
	'v', 255, 'w', WF_NOI, 't', 2, 'd', 0x80, '+', 0, 'w', WF_PUL, 'l', 0x80, 't', 12, 'v', 0, 0, 0	// bass drum
};


const struct track trackdata = {
	{
		{12*3+0,	1,	{'i',	0},	{0xff,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*3+0,	1,	{0,	0},	{0,	0}},
		{12*3+3,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*3+3,	1,	{0,	0},	{0,	0}},
		{12*3+2,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*3+7,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*3+3,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*3+2,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*2+10,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*3+0,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*3+0,	1,	{0,	0},	{0,	0}},
		{12*3+3,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*3+3,	1,	{0,	0},	{0,	0}},
		{12*3+7,	1,	{0,	0},	{0,	0}},
		{12*3+8,	1,	{0,	0},	{0,	0}},
		{12*3+5,	1,	{0,	0},	{0,	0}},
		{12*3+7,	1,	{0,	0},	{0,	0}},
		{12*3+3,	1,	{0,	0},	{0,	0}},
		{12*3+2,	1,	{0,	0},	{0,	0}},
		{12*2+0,	1,	{0,	0},	{0,	0}},
		{12*2+10,	1,	{0,	0},	{0,	0}},
		{12*2+7,	1,	{0,	0},	{0,	0}},
		{12*3+2,	1,	{0,	0},	{0,	0}},
	}
};
*/

void audiocb(void *userdata, Uint8 *buf, int len){
	int i;

	for(i = 0; i < len; i++){
		buf[i] = interrupthandler();
	}
}

int main(int argc, char **argv){
	SDL_AudioSpec requested, obtained;
	//FILE *ztlog;

	//ztlog = fopen("ztlog","w");

	//if (SDL_Init( SDL_INIT_AUDIO | SDL_INIT_JOYSTICK ) < 0){
	if(SDL_Init( SDL_INIT_AUDIO ) < 0){
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	//initjoystick();

	//fprintf(ztlog, "%i joysticks were found.\n\n", SDL_NumJoysticks() );
	//fprintf(ztlog, "The names of the joysticks are:\n");

	//int i;
	//for( i=0; i < SDL_NumJoysticks(); i++ ){
	//	fprintf(ztlog,"    %s\n", SDL_JoystickName(i));
	//}

	atexit(SDL_Quit);

	requested.freq = 16000;
	requested.format = AUDIO_U8;
	requested.samples = 256;
	requested.callback = audiocb;
	requested.channels = 1;

	// comment this out to run on grace
	//if(SDL_OpenAudio(&requested, &obtained) == -1){
	//	err(1, "SDL_OpenAudio");
	//}

    // Actually if we don't do error checking it just works on grace and
	//     // locally with sound :3
	SDL_OpenAudio(&requested, &obtained);

	fprintf(stderr, "freq %d\n", obtained.freq);
	fprintf(stderr, "samples %d\n", obtained.samples);

	initchip();
	initgui();

	if(argc != 2){
		loadfile("untitled.song");
	}else{
		loadfile(argv[1]);
	}

	SDL_PauseAudio(0);

	guiloop();
	
	return 0;
}
