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
#include "filetypes.h"
#include "hvl_replay.h"

// TODO: make hvl.c
 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < struct pineapple_tune *hvl_loadtune(struct *)  >                      .|
///  Gives you a struct pineapple_tune from a struct *hvl_tune.              .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
//struct pineapple_tune *hvl_loadtune(struct *hvl_tune) {
//}

int main(int argc, char **argv){
	//----------------------------------\\
	// parse those args :^)
	//----------------------------------//
	if(argc > 1){
		if(tune = lft_loadfile(argv[1])){
			fprintf(stderr, "loaded %s\n", argv[1]);
		//TODO make 48000 configurable as 'samplefreq'
		}else if(tune = hvl_LoadTune(argv[1], 48000, 4)){
			fprintf(stderr, "loading ahx/hvl...\n");
			fprintf(stderr, "loaded %s\n", argv[1]);
		}else{
			fprintf(stderr, "couldn't load %s\n", argv[1]);
			fprintf(stderr, "loading empty tune");
			tune = lft_loadfile("");
		}
	}else{
	//what happens if we load with no filename?
		// make an empty tune?
		//tune = pt_empty_tune();
		tune = lft_loadfile("");
	}

	if(sdl_init() == 0){
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
