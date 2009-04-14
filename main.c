/* vi:set ts=4 sts=4 sw=4: */
#include <stdio.h>
#include <SDL/SDL.h>

#ifndef WINDOWS
#include <err.h>
#endif

#include "stuff.h"

void audiocb(void *userdata, Uint8 *buf, int len){
	int i;

	for(i = 0; i < len; i++){
		buf[i] = interrupthandler();
	}
}

int main(int argc, char **argv){
	SDL_AudioSpec requested, obtained;

	if(SDL_Init( SDL_INIT_AUDIO ) < 0){
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

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
