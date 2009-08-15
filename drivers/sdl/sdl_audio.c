#include "../../pineapple.h"
#include <SDL/SDL.h>
#define FREQ 48000


/* SDL audioInit function */
u8 sdl_init(void){
	SDL_AudioSpec requested, obtained;

	fprintf(stderr, "Trying SDL....\n");

	if(SDL_Init( SDL_INIT_AUDIO ) < 0){
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	atexit(SDL_Quit);

	requested.freq = FREQ;
	requested.format = AUDIO_U8;
	requested.samples = 256;
	//requested.samples = config_param.buffersize;
	requested.channels = 1;
	requested.callback = lftSdlCallback;

	SDL_OpenAudio(&requested, &obtained);

	fprintf(stderr, "freq %d\n", obtained.freq);
	fprintf(stderr, "req. format %d\n", obtained.format);
	fprintf(stderr, "obtained format %d\n", obtained.format);
	fprintf(stderr, "samples %d\n", obtained.samples);

	return 0;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void lftSdlCallback(void* userdata,Uint8 *buf,int len) >              .|
///  SDLCallback function for LFT format.                                    .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void lftSdlCallback(void *userdata, Uint8 *buf, int len){
	int i;
	for(i = 0; i < len; i++){
		buf[i] = interrupthandler();
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void hvlSdlCallBack(void*,Uint8*,int) >                           .|
///  SDLCallback function for HVL/AHX format.                                .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
//void hvlSdlCallBack(struct hvl_tune *ht, uint8 *stream, int length){
//}
