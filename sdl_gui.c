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
#include "filetypes.h"

static int die = 0;
static SDL_Surface *screen;
static SDL_Surface *font = NULL;
static Uint32 boxcolor;

#define FONT_CW 10
#define FONT_CH 16
#define WINW 800
#define WINH 600
#define SP 2  //spacing
#define BOXW 1  //box width

#define FPS 30

// these aren't #define's because doing this in the preprocessor:
//     #define COLW = (WINW/3)-3
// made gcc complain
static int voffs;
static int colh;

//pineapple_tune tune;

int main(int argc, char **argv){
	int tick;

	//------------------------------------------------------------------\\
	// args
	//------------------------------------------------------------------//
	if(argc==1){
		tune = pt_empty_tune();
	}else{
		tune = lft_loadfile(argv[1]);
	}

	if(SDL_Init(SDL_INIT_VIDEO)!=0)
		return 1;

	voffs = FONT_CH+4;
	colh = WINH-voffs-SP-BOXW;

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

	//last_tick=SDL_GetTicks();
	while(!die){
		SDL_Event ev;
		tick = SDL_GetTicks();
		//int dt = tick - last_tick;
		//last_tick = tick;

		/* Handle GUI events */
		while(SDL_PollEvent(&ev)){
			switch(ev.type){
			case SDL_KEYDOWN:
				switch(currmode){
				case PM_NORMAL:
					handle_key_normal(&ev);
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

		//update_main(screen);
		if((SDL_GetTicks() - tick) < 1000/FPS){
			SDL_Delay( (1000/FPS) - (SDL_GetTicks() - tick));
		}
		gui_refresh();
	}

	SDL_Quit();
	return 0;
}

static void breakhandler(int a){
	die = 1;
}

static void draw_songed(const int x, const int y, const int w, const int h){
	int i, k, pos;
	char buf[1024];
	int j = 0;

	for(i=0; (i+y < h)
		//&&(i < h/FONT_CH)
		&&(j < tune->songlen); 
		i+=(FONT_CH+2))
	{
		int space = 0;
		pos = i/FONT_CH;
		snprintf(buf, sizeof(buf), "%02x| ", j);
		gui_text(x+SP,i+y+SP,buf,screen);
		for(k = 0; k < 4; k++){
			snprintf(buf, sizeof(buf),
				"%02x:%02x", tune->sng[j].track[k], tune->sng[j].transp[k]);
			gui_text(x+(space+=40),i+y+SP,buf,screen);
		}

		fprintf(stderr,"tune->songlen: %i\n", tune->songlen);
		fprintf(stderr,"i: %i\n", i);
		fprintf(stderr,"j: %i\n", j);

		// draw a box if selected
		if(songy==pos){
			gui_box(x,i,w,FONT_CH+(SP*2),boxcolor,screen);
		}
		j++;
	}
	gui_box(x,y,w,h,boxcolor,screen);
}

static void draw_tracked(const int x, const int y, const int w, const int h){
	gui_box(x,y,w,h,boxcolor,screen);
}

static void draw_instred(const int x, const int y, const int w, const int h){
	gui_box(x,y,w,h,boxcolor,screen);
}

static void draw_main(void){
	gui_text(2,2,"PINEAPPLEtRACKER",screen);
	draw_songed(SP,voffs,(23*FONT_CW)+(SP*2),colh);
	draw_tracked((23*FONT_CW)+(SP*4)+(BOXW*2),voffs,(18*FONT_CW)+(SP*2),
			colh);
	draw_instred((41*FONT_CW)+(SP*7)+(BOXW*4),voffs,(9*FONT_CW)+(SP*2),
			colh);
}

static void handle_key_normal(SDL_Event *ev){
	switch(ev->key.keysym.sym){
	case SDLK_KP_PLUS:
	case SDLK_PLUS:
	case SDLK_KP_MINUS:
	case SDLK_MINUS:
	break;
	case SDLK_h:
	case SDLK_LEFT:
		fprintf(stderr,"H\n");
		fprintf(stderr,"pos: %d, %d\n", songx, songy);
		act_mvleft();
		break;
	case SDLK_l:
	case SDLK_RIGHT:
		fprintf(stderr,"J\n");
		fprintf(stderr,"pos: %d, %d\n", songx, songy);
		act_mvright();
		break;
	case SDLK_k:
	case SDLK_UP:
		fprintf(stderr,"K\n");
		fprintf(stderr,"pos: %d, %d\n", songx, songy);
		act_mvup();
		break;
	case SDLK_j:
	case SDLK_DOWN:
		fprintf(stderr,"L\n");
		fprintf(stderr,"pos: %d, %d\n", songx, songy);
		act_mvdown();
		break;
	case SDLK_PAGEUP:
		act_bigmvup();
		break;
	case SDLK_PAGEDOWN:
		act_bigmvdown();
		break;
	case SDLK_F1:
	case SDLK_F2:
	case SDLK_F3:
	case SDLK_F4:
	case SDLK_F5:
	case SDLK_F6:
	case SDLK_F7:
	case SDLK_F8:
	case SDLK_F9:
	case SDLK_F10:
	case SDLK_F11:
	case SDLK_F12:
	case SDLK_PERIOD:
	case SDLK_DELETE:
	case SDLK_BACKSPACE:
	case SDLK_0:
	case SDLK_1:
	case SDLK_2:
	case SDLK_3:
	case SDLK_4:
	case SDLK_5:
	case SDLK_6:
	case SDLK_7:
	case SDLK_8:
	case SDLK_9:
	case SDLK_KP0:
	case SDLK_KP1:
	case SDLK_KP2:
	case SDLK_KP3:
	case SDLK_KP4:
	case SDLK_KP5:
	case SDLK_KP6:
	case SDLK_KP7:
	case SDLK_KP8:
	case SDLK_KP9:
	case SDLK_SPACE:
	case SDLK_TAB:
	case SDLK_ESCAPE:
	default:
		break;
	}
}

//static void update_main(SDL_Surface *screen){
	//unsigned int pos;

	//draw_songed(SP,voffs,230+(SP*2),colh);
	//draw_tracked(230+(SP*4),voffs,100,colh);
	//draw_instred(230+100*(SP*8),voffs,100,colh);
//}


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
	r.h = BOXW;
	SDL_FillRect(dst, &r, c);

	r.x = x;
	r.y = y + h - BOXW;
	r.w = w;
	r.h = BOXW;
	SDL_FillRect(dst, &r, c);

	r.x = x;
	r.y = y + BOXW;
	r.w = BOXW;
	r.h = h - (BOXW*2);
	SDL_FillRect(dst, &r, c);

	r.x = x + w - BOXW;
	r.y = y + BOXW;
	r.w = BOXW;
	r.h = h - (BOXW*2);
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
