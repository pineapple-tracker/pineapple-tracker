/* vi:set ts=8 sts=8 sw=8 noexpandtab: */
#include <stdio.h>

#include <SDL/SDL.h>

#ifndef WINDOWS
#include <err.h>
#endif

#ifdef JACK
#include <jack/jack.h>
#endif

#include "pineapple.h"
#include "musicchip_file.h"
#include "hvl_replay.h"

#define FREQ 48000

struct pineapple_tune *tune;

void sdl_callbackbuffer(void *userdata, Uint8 *buf, int len);
u8 (*audioInit)(void); //function that changes depending on which audiodriver is specified
void (*sdlCallback)(void); //function that changes depending on which type of file is loaded

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
	//requested.freq = 16000;
	//requested.format = AUDIO_S16SYS;
	requested.format = AUDIO_U8;
	requested.samples = 256;
	requested.channels = 1;
	requested.callback = sdl_callbackbuffer;
	//requested.userdata = tune;

	SDL_OpenAudio(&requested, &obtained);

	fprintf(stderr, "freq %d\n", obtained.freq);
	fprintf(stderr, "req. format %d\n", obtained.format);
	fprintf(stderr, "obtained format %d\n", obtained.format);
	fprintf(stderr, "samples %d\n", obtained.samples);

	return 0;
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < void sdl_callbackbuffer(void*,Uint8*,int) >                           .|
///  Called by SDL. Fills the dac buffer.                                    .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
void sdl_callbackbuffer(struct pineapple_tune *tune, Uint8 *buf, int len){
	for(int i = 0; i < len; i++){
		buf[i] = interrupthandler();
	}
}

 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < struct pineapple_tune *loadPineapple(char *fname) >                   .|
///  Loads the file specified on the command line, figures out what type of  .\
\\\ file it is and returns a pointer to a struct pineapple_tune.              `/
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
struct pineapple_tune *loadPineapple(char *fname) {
	struct pineapple_tune *tune;
}

int main(int argc, char **argv){
	if(argc != 2){
		loadfile("untitled.song");
	}else{
		loadfile(argv[1]);
	}

	initchip();
	initgui();
	
	SDL_PauseAudio(0);
	guiloop();

	SDL_Quit();
	
	return 0;
}
