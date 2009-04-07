#include <stdio.h>
#include <err.h>
#include <SDL/SDL.h>
#include <caca.h>

#include "stuff.h"

void audiocb(void *userdata, Uint8 *buf, int len) {
	int i;

	for(i = 0; i < len; i++) {
		buf[i] = interrupthandler();
	}
}

int main(int argc, char **argv){
	SDL_AudioSpec requested, obtained;

	int quit = 0;
	int xo, yo;

	//struct track track[256];
	//struct songline song[256];

	caca_display_t *dp;
	caca_canvas_t *cv;
	caca_canvas_t *pineapple;

	if(SDL_Init( SDL_INIT_AUDIO ) < 0){
		err(1, "Couldnt initialize SDL\n");
		exit(1);
	}

	cv = caca_create_canvas(80, 24);
	pineapple = caca_create_canvas(0, 0);
	if((cv == NULL) || (pineapple == NULL)){
		printf("failed to create canvas\n");
		return 1;
	}
	dp = caca_create_display(cv);
	caca_set_display_time(dp, 20000);
	if(dp == NULL){
		printf("Failed to create display\n");
		return 1;
	}

	caca_import_file(pineapple, "./pineapple", "");

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
	startplaysong(0);


	while(!quit)
	{
		caca_event_t ev;
		caca_set_color_ansi(cv, CACA_DEFAULT, CACA_DEFAULT);
		caca_clear_canvas(cv);
		xo = caca_get_canvas_width(cv);
		yo = caca_get_canvas_height(cv);
		//caca_blit(cv, 0, 0, pineapple, NULL);
		//caca_blit(cv, 55, 0, pineapple, NULL);
		caca_set_color_ansi(cv, caca_rand(0, 16), caca_rand(0, 16));
		caca_put_str(cv, (xo - strlen("pineapple player")) / 2, yo / 2, "pineapple player");
		caca_set_color_ansi(cv, caca_rand(0, 16), caca_rand(0, 16));
		caca_printf(cv, (xo - strlen("song pos ->   ")) / 2, (yo / 2) + 2, "song pos -> %x", songpos);
		//caca_clear_canvas(cv);
		
		/* note visualizer */
		/*if(track[0].line[0].note != 0){
			caca_put_str(cv, 0, 0, "!");
		}*/
		//readsong(songpos, ch, tmp);
			

    while(caca_get_event(dp, CACA_EVENT_ANY, &ev, 0))
    {
    	if(caca_get_event_type(&ev) & CACA_EVENT_KEY_PRESS)
    	{
				switch(caca_get_event_key_ch(&ev))
				{
					case 'q':
					case 'Q':
					case CACA_KEY_ESCAPE:
						quit = 1;
						break;
				}
			}
		}
		caca_refresh_display(dp);
	}
	caca_free_display(dp);
	caca_free_canvas(cv);
	return 0;
}
