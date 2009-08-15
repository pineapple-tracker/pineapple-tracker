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
#include "hvl_replay.h"



//struct pineapple_tune *tune;



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
		//what happens if we load with no filename?
		
		initinstrs();
		
		if(lft_loadfile(f,&tune) == 0){
			fprintf(stderr, "loaded %s\n", f);
		//TODO make 48000 configurable as 'samplefreq'
		}else if(hvl_LoadTune(f, 48000, 4) != NULL){
			fprintf(stderr, "loading ahx/hvl...\n");
			fprintf(stderr, "loaded %s\n", f);
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
