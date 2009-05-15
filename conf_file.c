#include "gui.h"
#include "pineapple.h"
#include "conf_file.h"
/*                   */
// ** CONFIG FILE ** //
/*                   */
FILE *pineapplefile;
int conf_open(){
	char variable[50];          //no variable names above 50 chars
	char value[50];					//array for right side
	//string fName = ".pineapplerc";
	pineapplefile = fopen(".pineapplerc", "r");

	if(pineapplefile == NULL){
		//keep default variables
	}
	else{
		while( fscanf( pineapplefile, "%s", variable)!=EOF){
			//parse stuff
			
		}
	}
return 0;
}
