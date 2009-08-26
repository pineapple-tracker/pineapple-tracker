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
#include "gui.h"

u8 (*sdl_init)(void);
pineapple_tune *importHvl(struct hvl_tune *ht);

// TODO: make hvl.c
 //\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/\\
\\\  < struct pineapple_tune *importHvl(struct hvl_tune *ht)  >              .|
///  Gives you a struct pineapple_tune from a struct *hvl_tune.              .\
 \\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\//
pineapple_tune *importHvl(struct hvl_tune *ht) {
	pineapple_tune *tune;
	tune = (pineapple_tune*) malloc(sizeof(pineapple_tune));
	if(!tune) {
		fprintf(stderr, "couldn't malloc pineapple_tune *tune!\n");
		return NULL;
	}

	tune->type = AHX;

	tune->songlen = ht->ht_PositionNr;
	
	tune->tracklen = ht->ht_TrackLength;
	for(int i = 0; i < tune->songlen; i++) {
		for(int j = 0; j < ht->ht_Channels; j++){
			tune->sng[i].track[j] = ht->ht_Positions[i].pos_Track[j];
			tune->sng[i].transp[j] = ht->ht_Positions[i].pos_Transpose[j];
		}
	}

	for(int i = 0; i < ht->ht_TrackNr; i++) {
		for(int j = 0; j < tune->tracklen; j++) {
			tune->trk[i].line[j].note = ht->ht_Tracks[i][j].stp_Note;
			tune->trk[i].line[j].instr = ht->ht_Tracks[i][j].stp_Instrument;
			tune->trk[i].line[j].cmd[0] = ht->ht_Tracks[i][j].stp_FX;
			//fprintf(stderr, "fx: %i \n", ht->ht_Tracks[i][j].stp_FX);
			tune->trk[i].line[j].param[0] = ht->ht_Tracks[i][j].stp_FXParam;
			tune->trk[i].line[j].cmd[1] = ht->ht_Tracks[i][j].stp_FXb;
			tune->trk[i].line[j].param[1] = ht->ht_Tracks[i][j].stp_FXbParam;
		}
	}

	tune->iedplonk = hvl_playNote;

	return tune;
}

int main(int argc, char **argv){
	//----------------------------------\\
	// parse those args :^)
	//----------------------------------//
	tune = (pineapple_tune*) malloc(sizeof(pineapple_tune));
	if(!tune) {
		fprintf(stderr, "couldn't malloc pineapple_tune *tune!\n");
		return 1;
	}

	initinstrs();
	hvl_InitReplayer();

	if(argc > 1){
		if((tune = lft_loadfile(argv[1]))){
			fprintf(stderr, "loaded %s\n", argv[1]);
			sdl_init = lft_sdl_init;
		//TODO make 48000 configurable as 'samplefreq'
		}else if((htTune = hvl_LoadTune(argv[1], 48000, 4))){
			fprintf(stderr, "loading ahx/hvl...\n");
			fprintf(stderr, "loaded %s\n", argv[1]);
			tune = importHvl(htTune);
			sdl_init = hvl_sdl_init;
			hvl_InitSubsong(htTune,0);
		}else{
			fprintf(stderr, "couldn't load %s\n", argv[1]);
			fprintf(stderr, "loading empty tune");
			tune = lft_loadfile("");
			sdl_init = lft_sdl_init;
		}
	}else{
	//what happens if we load with no filename?
		// make an empty tune?
		//tune = pt_empty_tune();
		/* FIXME: the function below segfaults! */
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
	free(tune);
	if(htTune)
		hvl_FreeTune(htTune);
	return 0;
}
