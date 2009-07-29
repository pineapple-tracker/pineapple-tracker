/*
 * (C) David Olofson, 2003, 2006
 *   modified by the pineapple team, 2009
 */
#include "sdl_gui.h"
#include "gui.h"
#include "pineapple.h"

SDL_Surface *screen;

int main(int argc, char **args){
	int last_tick;
	int die = 0;

	if(SDL_Init(SDL_INIT_VIDEO)!=0)
		return 1;

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(640,480,32,0);
	if(screen==NULL){
		fprintf(stderr,"video init failed");
		fprintf(stderr,SDL_GetError());
		SDL_Quit();
		return 1;
	}else{
		fprintf(stdout,"initialized the surface\n");
	}

	SDL_EnableKeyRepeat(250, 25);

	last_tick=SDL_GetTicks();
	while(!die){
		SDL_Event ev;
		int tick = SDL_GetTicks();
		int dt = tick - last_tick;
		last_tick = tick;

		/* Handle GUI events */
		while(SDL_PollEvent(&ev)){
			switch(ev.type){
			case SDL_KEYDOWN:
				switch(currmode){
				case PM_NORMAL:
					//handle_key(&ev);
					break;
				case PM_VISUAL:
					break;
				case PM_VISUALLINE:
					break;
				case PM_CMDLINE:
					break;
				case PM_INSERT:
					break;
				case PM_JAMMER:
					break;
				}
				break;
			case SDL_QUIT:
				break;
			default:
				break;
			}
		}

		update_main(screen, dt);

		SDL_Delay(10);
	}

	SDL_Quit();
	return 0;
}

static void ppp_songtab(void){
	Uint32 hlc = SDL_MapRGB(screen->format, 255, 0, 255);
	gui_box(0,0, 220,480, hlc,screen);
}

static void draw_main(void){
	ppp_songtab();
}

static void update_main(SDL_Surface *screen, int dt){
}

SDL_Surface *gui_load_image(const char *fn){
	SDL_Surface *cvt;
	SDL_Surface *img = SDL_LoadBMP(fn);
	if(!img)
		return NULL;
	cvt = SDL_DisplayFormat(img);
	SDL_FreeSurface(img);
	return cvt;
}

void gui_box(int x, int y, int w, int h, Uint32 c, SDL_Surface *dst){
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = 1;
	SDL_FillRect(dst, &r, c);

	r.x = x;
	r.y = y + h - 1;
	r.w = w;
	r.h = 1;
	SDL_FillRect(dst, &r, c);

	r.x = x;
	r.y = y + 1;
	r.w = 1;
	r.h = h - 2;
	SDL_FillRect(dst, &r, c);

	r.x = x + w - 1;
	r.y = y + 1;
	r.w = 1;
	r.h = h - 2;
	SDL_FillRect(dst, &r, c);

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	//gui_dirty(&r);
}

void gui_bar(int x, int y, int w, int h, Uint32 c, SDL_Surface *dst){
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	SDL_FillRect(dst, &r, SDL_MapRGB(dst->format, 0, 0, 0));
	gui_box(x, y, w, h, c, dst);
}
