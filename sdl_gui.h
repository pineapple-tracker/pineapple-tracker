/*
 * (C) David Olofson, 2006
 *   modified by the pineapple team, 2009
 */


#include <SDL/SDL.h>
#include <signal.h>
#include <stdio.h>

static void breakhandler(int a);

/*
 * Low level GUI stuff
 */

/* Load and convert image */
SDL_Surface *gui_load_image(const char *fn);
static void draw_main();

/* Add a dirtyrect */
void gui_dirty(SDL_Rect *r);

/* Update all dirty areas */
void gui_refresh(void);

/* Draw a hollow box */
void gui_box(int x, int y, int w, int h, Uint32 c, SDL_Surface *dst);

/* Draw a black box with a colored outline */
void gui_bar(int x, int y, int w, int h, Uint32 c, SDL_Surface *dst);

/* Render text */
void gui_text(int x, int y, const char *txt, SDL_Surface *dst);

/* Render an oscilloscope */
//void gui_oscilloscope(Sint32 *buf, int bufsize,
//		int start, int x, int y, int w, int h,
//		SDL_Surface *dst);

/*
 * High level GUI stuff
 */
/*int gui_open(SDL_Surface *screen);
void gui_close(void);

void gui_tempo(int v);
void gui_songpos(int v);
void gui_songedit(int pos, int ppos, int track, int editing);
void gui_songselect(int x1, int y1, int x2, int y2);
void gui_status(int playing, int editing, int looping);
void gui_message(const char *message, int curspos);
void gui_draw_activity(int dt);
void gui_activity(int trk);

void gui_draw_screen(GUI_pages page);*/
static void update_main(SDL_Surface *screen, int dt);
