#include "pineapple.h"

//---------------------------------------------------------------------------\\
// pt
//---------------------------------------------------------------------------//
pineapple_tune *pt_empty_tune();


//---------------------------------------------------------------------------\\
// lft
//---------------------------------------------------------------------------//
pineapple_tune *lft_loadfile(char *fname);
void lft_savefile(char *fname);
void lft_saveinstrument(char *fname);
int lft_loadinstrument(char *fname);
