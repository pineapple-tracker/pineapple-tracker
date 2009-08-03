//just gonna see if i can display a hively position list and all the hively tracks and make it scroll

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ncurses.h>
#include <SDL/SDL.h>
#include "hvl_replay.h"

#define FREQ 48000
#define HIVELY_LEN FREQ/50
#define OUTPUT_LEN 4096

int16 hivelyLeft[HIVELY_LEN], hivelyRight[HIVELY_LEN];
size_t hivelyIndex;

void mix_and_play( struct hvl_tune *ht, uint8 *stream, int length );

BOOL init( void )
{
	//uint32 i;
	SDL_AudioSpec wanted;

	if(SDL_Init(SDL_INIT_AUDIO)< 0) {
		printf("Could not initialize SDL: %s\n", SDL_GetError());
		SDL_Quit();
		return FALSE;
	}

	wanted.freq = FREQ; 
	wanted.format = AUDIO_S16SYS; 
	wanted.channels = 2; /* 1 = mono, 2 = stereo */
	wanted.samples = OUTPUT_LEN; // HIVELY_LEN;

	wanted.callback = (void*) mix_and_play;
	wanted.userdata = tune;

	if(SDL_OpenAudio(&wanted, NULL) < 0) {
		printf("Failed to open audio device.\n");
		SDL_Quit();
		return FALSE;
	}

	return TRUE;
}

void mix_and_play( struct hvl_tune *ht, uint8 *stream, int length )
{
	int16 *out;
	int i;
	size_t streamPos = 0;
	length = length >> 1;

	uint32 samples;
	samples = tune->ht_Frequency/50/ht->ht_SpeedMultiplier;

	if(tune && play) {
		// Mix to 16bit interleaved stereo
		out = (int16*) stream;
		// Flush remains of previous frame
		for(i = hivelyIndex; i < (HIVELY_LEN); i++) {
			out[streamPos++] = hivelyLeft[i];
			out[streamPos++] = hivelyRight[i];
		}

		while(streamPos < length) {
			hvl_DecodeFrame( tune, (int8 *) hivelyLeft, (int8 *) hivelyRight, 2 );
			for(i = 0; i < (HIVELY_LEN) && streamPos < length; i++) {
				out[streamPos++] = hivelyLeft[i];
		 		out[streamPos++] = hivelyRight[i];
			}
		}
		hivelyIndex = i;
	} else if(tune && plonked) {
		// Mix to 16bit interleaved stereo
		out = (int16*) stream;
		// Flush remains of previous frame
		for(i = hivelyIndex; i < (HIVELY_LEN); i++) {
			out[streamPos++] = hivelyLeft[i];
			out[streamPos++] = hivelyRight[i];
		}

		while(streamPos < length) {
			hvl_playNote( tune, (int8 *) hivelyLeft, (int8 *) hivelyRight, 2, &tune->ht_Voices[0]);
			for(i = 0; i < (HIVELY_LEN) && streamPos < length; i++) {
				out[streamPos++] = hivelyLeft[i];
		 		out[streamPos++] = hivelyRight[i];
			}
		}
		hivelyIndex = i;
	}
}

int main(int argc, char *argv[])
{
	hivelyIndex = HIVELY_LEN;
	if(argc < 2){
		printf( "Usage: play_hvl <tune>\n" );
		return 0;
	}

	if(init()){
		hvl_InitReplayer();
		tune = hvl_LoadTune(argv[1], FREQ, 4);
		if(tune){
			//BOOL done;
			uint32 gotsigs;

			hvl_InitSubsong( tune, 0 );
			initgui();
			play = 0; 
			SDL_PauseAudio(0);
			guiloop();
		}
	}

	SDL_PauseAudio(1);
	hvl_FreeTune(tune);
	SDL_Quit();
	return 0;
}
