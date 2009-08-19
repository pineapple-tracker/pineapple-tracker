//--------------------------------------------------\\
// pineapple tracker song format
//--------------------------------------------------//

#include "filetypes.h"

pineapple_tune *pt_empty_tune(void){
	pineapple_tune *t;

	t->type = "pt";
	t->filename = "untitled";
	t->callbacktime = 240;
	t->tempo = 120;
	t->comment = "";
	t->trackpos = 0;
	t->songpos = 0; //ht_PosNr
	t->songlen = sizeof(t->sng) / sizeof(t->sng[0]);

	t->instrx = 0,
	t->instry = 0,
	t->instroffs = 0;

	t->songx = 0,
	t->songy = 0,
	t->songoffs = 0;

	t->trackx = 0,
	t->tracky = 0,
	t->trackoffs = 0;
	t->currtrack, t->currinstr, t->currtab = 0;
	t->saved = 1;

	return t;
}

