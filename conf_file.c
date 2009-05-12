#include <gui.h>
#include <pineapple.h>
/*									*/
// ** CONFIG FILE ** //
/*									*/
FILE *pineapplefile;
int conf_open()
{
	char word[50];		//no variable names above 50 chars
	string fName = ".pineapplerc";
	pineapplefile = fopen(fName, "r");
	if(pineapplefile == NULL){
		//keep default variables
	}
	else{
		while( fscanf( pineapplefile, "%s", word)!=EOF){
			//parse stuff
		}
	}
}
