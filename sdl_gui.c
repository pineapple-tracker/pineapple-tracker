/*
 * (C) David Olofson, 2003, 2006
 *   modified by the pineapple team, 2009
 */
#include <SDL/SDL.h>
#include <signal.h>
#include <stdio.h>

#include "sdl_gui.h"
#include "gui.h"
#include "pineapple.h"

static int die = 0;
static SDL_Surface *screen;
static SDL_Surface *font = NULL;
static Uint32 boxcolor;

#define FONT_CW 10
#define FONT_CH 16
#define WINW 800
#define WINH 600
#define SP 4  //spacing

// these aren't #define's because doing this in the preprocessor:
//     #define COLW = (WINW/3)-3
// made gcc complain
static int voffs;
static int colw;
static int colh;


int main(int argc, char *args[]){
	int last_tick;

	if(SDL_Init(SDL_INIT_VIDEO)!=0)
		return 1;

	voffs = FONT_CH+4;
	colw = (WINW/3)-3;
	colh= WINH-voffs-1;

	atexit(SDL_Quit);
	signal(SIGTERM,breakhandler);
	signal(SIGINT,breakhandler);

	screen = SDL_SetVideoMode(WINW,WINH,32,SDL_SWSURFACE);
	if(screen==NULL){
		fprintf(stderr,"video init failed");
		SDL_Quit();
		return 1;
	}else{
		fprintf(stdout,"initialized the surface\n");
	}

	boxcolor = SDL_MapRGB(screen->format, 255, 0, 255);
	SDL_WM_SetCaption("pineapple-tracker","pineapple.ico");
	if(gui_open(screen) != 0){
		fprintf(stderr, "Couldn't start GUI!\n");
		SDL_Quit();
		return -1;
	}

	draw_main();
	SDL_Flip(screen);

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
				die = 1;
				break;
			default:
				break;
			}
		}

		//update_main(screen, dt);

		gui_refresh();

		// try to take up less cpu
		SDL_Delay(20);
	}

	SDL_Quit();
	return 0;
}

static void draw_songed(int x, int y, int w, int h){
	int i, line;
	char buf[1024];

	//void gui_box(int x, int y, int w, int h, Uint32 c, SDL_Surface *dst){
	for(i=0; i+y<h; i+=(FONT_CH+2)){
		snprintf(buf, sizeof(buf),
			"%02x:00 00:00 00:00 00:00", i/FONT_CH);
		gui_text(x+SP,i+y+SP,buf,screen);
	}
	gui_box(x,y,w,i+(SP*2),boxcolor,screen);
}

static void draw_tracked(void){
	gui_box(colw+3,voffs,colh,colw,boxcolor,screen);
}

static void draw_instred(void){
	gui_box(colw*2+6,voffs,colh,colw,boxcolor,screen);
}

static void draw_main(void){
	gui_text(2,2,"PINEAPPLEtRACKER",screen);
	draw_songed(SP,voffs,230+SP,colh);
	draw_tracked();
	draw_instred();
}

static void update_main(SDL_Surface *screen, int dt){
}

static void breakhandler(int a){
	die = 1;
}


// \\ // \\ // \\  //
 // gui_* functions //
  // \\ // \\ // \\  //

void gui_bar(int x, int y, int w, int h, Uint32 c, SDL_Surface *dst){
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	SDL_FillRect(dst, &r, SDL_MapRGB(dst->format, 0, 0, 0));
	gui_box(x, y, w, h, c, dst);
}

void gui_box(int x, int y, int w, int h, Uint32 c, SDL_Surface *dst){
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = SP;
	SDL_FillRect(dst, &r, c);

	r.x = x;
	r.y = y + h - SP;
	r.w = w;
	r.h = SP;
	SDL_FillRect(dst, &r, c);

	r.x = x;
	r.y = y + SP;
	r.w = SP;
	r.h = h - (SP*2);
	SDL_FillRect(dst, &r, c);

	r.x = x + w - SP;
	r.y = y + SP;
	r.w = SP;
	r.h = h - (SP*2);
	SDL_FillRect(dst, &r, c);

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	//gui_dirty(&r);
}

void gui_close(void){
	SDL_FreeSurface(font);
	font = NULL;
}

SDL_Surface *gui_load_image(const char *fn){
	SDL_Surface *cvt;
	SDL_Surface *img = SDL_LoadBMP(fn);
	if(!img){
		return NULL;
	}
	cvt = SDL_DisplayFormat(img);
	SDL_FreeSurface(img);
	return cvt;
}

int gui_open(SDL_Surface *scrn){
	screen = scrn;
	font = gui_load_image("font.bmp");
	if(!font){
		fprintf(stderr, "Couldn't load font!\n");
		return 1;
	}
	SDL_EnableKeyRepeat(250, 25);
	//memset(activity, 0, sizeof(activity));
	return 0;
}

void gui_refresh(void){
	SDL_Flip(screen);
}

void gui_text(int x, int y, const char *txt, SDL_Surface *dst){
	int sx = x;
	int sy = y;
	const char *stxt = txt;
	int highlights = 0;
	SDL_Rect sr;
	sr.w = FONT_CW;
	sr.h = FONT_CH;
	while(*txt){
		int c = *txt++;
		switch(c){
		  case 0:	/* terminator */
			break;
		  case '\n':	/* newline */
			x = sx;
			y += FONT_CH;
			break;
		  case '\t':	/* tab */
			x -= sx;
			x += 8 * FONT_CW;
			x %= 8 * FONT_CW;
			x += sx;
			break;
		  case '\001':	/* red highlight */
		  case '\002':	/* green highlight */
		  case '\003':	/* yellow highlight */
		  case '\004':	/* blue highlight */
		  case '\005':	/* purple highlight */
		  case '\006':	/* cyan highlight */
		  case '\007':	/* white highlight */
			highlights = 1;
			if(*txt == '\001'){
				txt += 2;
			}
			break;
		  case '\021':	/* red bullet */
		  case '\022':	/* green bullet */
		  case '\023':	/* yellow bullet */
		  case '\024':	/* blue bullet */
		  case '\025':	/* purple bullet */
		  case '\026':	/* cyan bullet */
		  case '\027':	/* white bullet */
		  {
			SDL_Rect r;
			int hlr = c & 1 ? 255 : 0;
			int hlg = c & 2 ? 255 : 0;
			int hlb = c & 4 ? 255 : 0;
			Uint32 hlc = SDL_MapRGB(dst->format, hlr, hlg, hlb);
			r.x = x;
			r.y = y;
			r.w = FONT_CW;
			r.h = FONT_CH;
			SDL_FillRect(dst, &r,
					SDL_MapRGB(dst->format, 0, 0, 0));
			//gui_dirty(&r);
			r.x = x + 2;
			r.y = y + 2;
			r.w = FONT_CW - 6;
			r.h = FONT_CH - 6;
			SDL_FillRect(dst, &r, hlc);
			x += FONT_CW;
			break;
		  }
		  default:	/* printables */
		  {
			SDL_Rect dr;
			if(c < ' ' || c > 127)
				c = 127;
			c -= 32;
			sr.x = (c % (font->w / FONT_CW)) * FONT_CW;
			sr.y = (c / (font->w / FONT_CW)) * FONT_CH;
			dr.x = x;
			dr.y = y;
			SDL_BlitSurface(font, &sr, dst, &dr);
			//gui_dirty(&dr);
			x += FONT_CW;
			break;
		  }
		}
	}
	if(!highlights){
		return;
	}
	x = sx;
	y = sy;
	txt = stxt;
	while(*txt){
		int c = *txt++;
		switch(c){
		  case 0:	/* terminator */
			break;
		  case '\n':	/* newline */
			x = sx;
			y += FONT_CH;
			break;
		  case '\t':	/* tab */
			x -= sx;
			x += 8 * FONT_CW;
			x %= 8 * FONT_CW;
			x += sx;
			break;
		  case '\001':	/* red highlight */
		  case '\002':	/* green highlight */
		  case '\003':	/* yellow highlight */
		  case '\004':	/* blue highlight */
		  case '\005':	/* purple highlight */
		  case '\006':	/* cyan highlight */
		  case '\007':	/* white highlight */
		  {
			int hlr = c & 1 ? 255 : 0;
			int hlg = c & 2 ? 255 : 0;
			int hlb = c & 4 ? 255 : 0;
			Uint32 hlc = SDL_MapRGB(screen->format, hlr, hlg, hlb);
			int hlw = 1;
			if(*txt == '\001'){
				hlw = txt[1];
				txt += 2;
			}
			gui_box(x - 2, y - 2,
					FONT_CW * hlw + 2, FONT_CH + 2,
					hlc, dst);
			break;
		  }
		  default:	/* printables */
		  {
			x += FONT_CW;
			break;
		  }
		}
	}
}
