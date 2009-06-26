#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOD_NO_NOTE		63
#define MOD_NO_SAMPLE	31

struct sample_header {
	char name[22]; //22 bytes
	unsigned short length; //2 bytes
	unsigned char finetune; //1 byte
	unsigned char vol; //1 byte
	unsigned short loopstart; //2 bytes
	unsigned short looplength; //2 bytes
	signed char *smpdata; //1 byte
};

//sizeof(sample_header) == 31 bytes?

struct mod_header {
	char name[20];
	struct sample_header sample[31];
	unsigned char order[128];
	unsigned char **pattern;
	unsigned char orderCount;
	unsigned char patternCount;
};

int main(int argc, char **argv){
	FILE *modfile;
	struct mod_header modheader;
	int i;
	int trash;
	unsigned char highestPattern = 0;

	const unsigned short octave1Period[12] = {
		856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453
	};

	unsigned short periodTable[12*5];
	unsigned char octave,note;

	for(octave = 0; octave < 5; octave++) {
		for(note = 0; note < 12; note++) {
			//*2 to get us into octave 0, then divide by 2 for each octave from there
			periodTable[octave*12 + note] = (octave1Period[note]*2) >> octave;
		}
	}

	/* ~opening the modfile~ */

	modfile = fopen(argv[1], "rb");

	if(!modfile) {
		printf("Couldn't open file!\n");
		return 1;
	}

	fseek(modfile, 0, SEEK_SET);
	fread(modheader.name, 20, 1, modfile);
	modheader.name[19] = '\0';

	/* ~ load samples ~*/
	
	for(i = 0; i < 31; i++) {
		fread(&modheader.sample[i], 30, 1, modfile);
		modheader.sample[i].smpdata = NULL;

		modheader.sample[i].length = ( ((modheader.sample[i].length & 0xff) << 8) |
										(modheader.sample[i].length >> 8) );

		modheader.sample[i].loopstart = ( ((modheader.sample[i].loopstart & 0xff) << 8) |
											(modheader.sample[i].loopstart >> 8) );

		modheader.sample[i].looplength = ( ((modheader.sample[i].looplength & 0xff) << 8) |
											(modheader.sample[i].looplength >> 8) );
	}

	/*~ load orders ~*/

	fread(&modheader.orderCount, 1, 1, modfile);

	fread(&trash, 1, 1, modfile); //useless byte

	fread(&modheader.order, 128, 1 , modfile);

	/*~ load patterns ~*/

	for(i = 0; i < modheader.orderCount; i++) {
		if(modheader.order[i] > highestPattern)
			highestPattern = modheader.order[i];
	}
	
	modheader.patternCount = highestPattern + 1;

	fread(&trash, 4, 1, modfile); //reading the M.K. string

	modheader.pattern = malloc(sizeof(modheader.patternCount));

	if(!modheader.pattern){
		printf("out of memory!\n");
	}

	//initialize modheader.pattern to 0
	//memset(modheader.pattern, 0, modheader.patternCount*sizeof(unsigned char*));

	unsigned char curPattern, row, column;

	for(curPattern = 0; curPattern < modheader.patternCount; curPattern++) {
		modheader.pattern[curPattern] = malloc(1024);
		if(!modheader.pattern[curPattern]){
			printf("out of memory!\n");
		}
		
		//initialize to 0
		//memset(modheader.pattern[curPattern], 0, 1024);

		for(row = 0; row < 64; row++) {
			unsigned char cell[4];
			unsigned char sample;
			unsigned short period;
			unsigned char effect;
			unsigned char param;

			fread(cell, 4, 1, modfile);

			sample = (cell[0] & 0xf0) | (cell[2] >> 4);
			period = cell[1] | ((cell[0] & 0xf) << 8);
			effect = cell[2] & 0xf;
			param = cell[3];

			//looping through the period table
			unsigned char closestNote = 0; 
			unsigned short closestDist = 0xffff; //make sure the first comparison sets the closet note
			unsigned short newDist;

			if(period == 0) {
				closestNote = MOD_NO_NOTE; //period 0 is no note
			}else {
				for(i = 0; i < 12*5; i++) {
					newDist = abs(period = periodTable[i]);
					if(newDist < closestDist){
						closestNote = (unsigned char)i;
						closestDist = newDist;
					}
				}
			}

			if(sample == 0) {
				sample = MOD_NO_SAMPLE;
			}else {
				sample -= 1;
			}

			//now that we have our note, we can store the data in our new pattern
			//calculate the address of the cell to output to
			// rowoffset = row * 4 columns per row * 4 bytes per cell
			// columnoffset = column * 4 bytes per cell
			
			unsigned char *outCell = &modheader.pattern[curPattern][row*4*4 + column*4];
			outCell[0] = closestNote;
			outCell[1] = sample;
			outCell[2] = effect;
			outCell[3] = param;
			
		
			
		}
	}

	
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	//done reading modfile
	fclose(modfile);
		
	printf("%s\n", modheader.name);

	for(i = 0; i < 31; i++)	{
		printf("%i: %s\n", i, modheader.sample[i].name);
		printf("%i: length:%i\n", i, modheader.sample[i].length);
	}

	printf("highest pattern: %i\n", highestPattern);

	printf("orderCount: %i\n",  modheader.orderCount);

	printf("patternCount: %i\n", modheader.patternCount);
	

	return 0;
}
