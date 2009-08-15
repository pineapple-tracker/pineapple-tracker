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
#include "lft.h"


//struct pineapple_tune *tune;

/* SDL audioInit function */
/*u8 sdl_init(void){
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
	requested.callback = sdl_callbackbuffer;

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
void sdl_callbackbuffer(void *userdata, Uint8 *buf, int len){
	for(int i = 0; i < len; i++){
		buf[i] = interrupthandler();
	}
}
*/



// TODO: make hvl.c
 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < struct pineapple_tune *hvl_loadtune(struct *)  >                      .|
///  Gives you a struct pineapple_tune from a struct *hvl_tune.              .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
//struct pineapple_tune *hvl_loadtune(struct *hvl_tune) {
//}


int main(int argc, char **argv){
	char * f;
	f = "";

	if(sdl_init() == 0){
		//----------------------------------\\
		// parse those args :^)
		//----------------------------------//
		if(argc > 1){
			f = argv[1];
		}

		if(lft_loadfile(f,&tune) == 0){
			fprintf(stderr, "loaded %s\n", f);
		//}else if(hvl_loadfile(filename, FREQ, 4) != NULL){
			// ????
		}else{
			fprintf(stderr, "couldn't load %s\n", f);
		}

		//----------------------------------\\
		// begin image&sound
		//----------------------------------//
		initchip();
		initgui();
		
		SDL_PauseAudio(0);

		//----------------------------------\\
		// actually start the program
		//----------------------------------//
		eventloop();

		SDL_Quit();
	}

	return 0;
}
