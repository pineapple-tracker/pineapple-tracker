/* vi:set ts=8 sts=8 sw=8 noexpandtab: */
#include <stdio.h>
#include <SDL/SDL.h>
#include <caca.h>
#ifndef WINDOWS
#include <err.h>
#endif

#include "pineapple.h"
#include "musicchip_file.h"

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
	int i, j, k;
	int meter[4];

	static char chars[10] =
	{
			'+', '-', '*', '#', 'X', '@', '%', '$', 'M', 'W'
	};

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

	requested.freq = 48000;
	requested.format = AUDIO_U8;
	requested.samples = 4096;
	requested.callback = audiocb;
	requested.channels = 1;

	if(SDL_OpenAudio(&requested, &obtained) == -1){
		err(1, "SDL_OpenAudio");
	}

	initchip();

	loadfile(argv[1]);

	SDL_PauseAudio(0);
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
		caca_blit(cv, 55, 0, pineapple, NULL);
		caca_set_color_ansi(cv, caca_rand(0, 16), caca_rand(0, 16));
		caca_put_str(cv, (xo - strlen("pineapple player")) / 2, (yo / 2) - 5, "pineapple player");
		caca_set_color_ansi(cv, caca_rand(0, 16), caca_rand(0, 16));
		caca_printf(cv, (xo - strlen("song pos ->   ")) / 2, (yo / 2) - 3, "song pos -> %x", songpos);
		
		for(i = 0; i < 4; i ++)
			meter[i] = (osc[i].volume*20)/255;
		/* note visualizer */
		i = 0;
		for(j = 0; j < 25; j=j+6){
				for(k = 0; k < 4; k++){
				caca_draw_line(cv, (((xo/2)+10)-j)-k, yo, (((xo/2)+10)-j)-k, yo - meter[i], 
					chars[caca_rand(0, 9)]);
				}
			i++;
		}

		for(i = 0; i < 4; i ++)
			caca_printf(cv, 0, i, "%0x", osc[i].volume);

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
	silence();
	caca_free_display(dp);
	caca_free_canvas(cv);
	return 0;
}
