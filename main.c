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
#include "gui.h"
#include "lft.h"
#include "hvl_replay.h"


struct pineapple_tune *importHvl(struct hvl_tune *ht);

// TODO: make hvl.c
 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < struct pineapple_tune *importHvl(struct hvl_tune *ht)  >                      .|
///  Gives you a struct pineapple_tune from a struct *hvl_tune.              .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
struct pineapple_tune *importHvl(struct hvl_tune *ht) {
	struct pineapple_tune *tune;
	tune = (struct pineapple_tune*) malloc(sizeof(struct pineapple_tune));
	if(!tune) {
		fprintf(stderr, "couldn't malloc pineapple_tune *tune!\n");
		return 1;
	}

	tune->songlen = ht->ht_PositionNr;
	printf("songlen: %i\n", tune->songlen);
		

	return tune;
}


int main(int argc, char **argv){
	char * f;
	f = "";

	if(sdl_init() == 0){
		//----------------------------------\\
		// parse those args :^)
		//----------------------------------//
		if(argc > 1){
			//what happens if we load with no filename?
			f = argv[1];
		}

		tune = (struct pineapple_tune*) malloc(sizeof(struct pineapple_tune));
		if(!tune) {
			fprintf(stderr, "couldn't malloc pineapple_tune *tune!\n");
			return 1;
		}

		//tune->songlen = 0;
		
		initinstrs();
		
		if(lft_loadfile(f,tune) == 0){
			fprintf(stderr, "loaded %s\n", f);
		//TODO make 48000 configurable as 'samplefreq'
		}else if((htTune = hvl_LoadTune(f, 48000, 4)) != NULL){
			fprintf(stderr, "loading ahx/hvl...\n");
			fprintf(stderr, "loaded %s\n", f);
			tune = importHvl(htTune);
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
