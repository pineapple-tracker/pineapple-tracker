#include "../../pineapple.h"
#include "../../hvl_replay.h"
#include <SDL/SDL.h>


/* lft SDL audioInit function */
u8 lft_sdl_init(void){
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
	fprintf(stderr, "req. format %d\n", requested.format);
	fprintf(stderr, "obtained format %d\n", obtained.format);
	fprintf(stderr, "samples %d\n", obtained.samples);

	return 0;
}

/* hvl SDL audioInit function */
u8 hvl_sdl_init(void){
	SDL_AudioSpec requested, obtained;

	fprintf(stderr, "Trying SDL....\n");

	if(SDL_Init( SDL_INIT_AUDIO ) < 0){
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	atexit(SDL_Quit);

	requested.freq = FREQ;
	requested.format = AUDIO_S16SYS;
	requested.samples = 256;
	//requested.samples = config_param.buffersize;
	requested.channels = 2;
	requested.callback = (void*) hvlSdlCallBack;

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
\\\  < void hvlSdlCallBack(void*,Uint8*,int) >                               .|
///  SDLCallback function for HVL/AHX format.                                .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void hvlSdlCallBack(struct hvl_tune *ht, u8 *stream, int length){
  int16 *out;
  int i;
  size_t streamPos = 0;
  length = length >> 1;

  if(htTune && playsong) {
    // Mix to 16bit interleaved stereo
    out = (int16*) stream;
    // Flush remains of previous frame
    for(i = hivelyIndex; i < (HIVELY_LEN); i++) {
      out[streamPos++] = htTune->hivelyLeft[i];
      out[streamPos++] = htTune->hivelyRight[i];
    }

    while(streamPos < length) {
		hvl_DecodeFrame( htTune, (int8 *) htTune->hivelyLeft, (int8 *) htTune->hivelyRight, 2 );
		for(i = 0; i < (HIVELY_LEN) && streamPos < length; i++) {
		      out[streamPos++] = htTune->hivelyLeft[i];
		      out[streamPos++] = htTune->hivelyRight[i];
		}
    }
    hivelyIndex = i;
  }
}

