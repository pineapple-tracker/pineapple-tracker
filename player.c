#include <stdio.h>
#include <err.h>
#include <SDL/SDL.h>

#include "stuff.h"

void audiocb(void *userdata, Uint8 *buf, int len) {
	int i;

	for(i = 0; i < len; i++) {
		buf[i] = interrupthandler();
	}
}

int main(int argc, char **argv){
	SDL_AudioSpec requested, obtained;

	if(SDL_Init( SDL_INIT_AUDIO ) < 0){
		err(1, "Couldnt initialize SDL\n");
		exit(1);
	}

	atexit(SDL_Quit);

	requested.freq = 16000;
	requested.format = AUDIO_U8;
	requested.samples = 256;
	requested.callback = audiocb;
	requested.channels = 1;

	if(SDL_OpenAudio(&requested, &obtained) == -1){
		err(1, "SDL_OpenAudio");
	}

	initchip();
	//initgui();

	loadfile(argv[1]);

	SDL_PauseAudio(0);
	//playmode = PM_PLAY;
	silence();
	startplaysong(1);
	for(;;){}

	return 0;
}
