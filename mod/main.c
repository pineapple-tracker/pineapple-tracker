#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>

#define MOD_NO_NOTE	63
#define MOD_NO_SAMPLE	31

#define FREQ 44100

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

double smp_index = 0;
double note = 65.41; //c2

int tick = 6;

int currpatt;
int currrow;

SDL_AudioSpec requested, obtained;

//static char *notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-",
//				"G#", "A-", "A#", "H-"};

struct sample_header {
	u8 name[22]; //22 bytes
	u16 length; //2 bytes
	u8 finetune; //1 byte
	u8 vol; //1 byte
	u16 loopstart; //2 bytes
	u16 looplength; //2 bytes
	char *smpdata; //1 byte
};

struct pattern_entry {
	u16 period;
	u8 sample;
	u8 effect;
	u8 param;
};

struct pattern {
	struct pattern_entry pattern_entry[64][4];
};

struct mod_header {
	char name[20];
	struct sample_header sample[31];
	u8 order[128];
	//u8 **pattern;
	struct pattern *patterns;
	u8 orderCount;
	u8 patternCount;
};

struct mod_header modheader;

void play(void){
	if(tick == 0)
		update_row();
	else
		tick--;
}

void update_row(void){
}

void callback(void *data, Uint8 *buf, int len){
	int i;
	s8 *out;
	u32 realLength = (modheader.sample[5].length) * 2;
	//fprintf(stderr, "len: %i\n", len);
	out = (s8*) buf;
	//fprintf(stderr, "reallength: %i\n", realLength);
	//fprintf(stderr, "smp_index: %f\n", smp_index);
	//if(smp_index < realLength){
		//for(i = 0; i < realLength; i ++){
		for(i = 0; i < len; i++){
			if(obtained.format == AUDIO_U8)
				out[i] = (modheader.sample[5].smpdata[(int)smp_index]) + 128;
			else
				out[i] = modheader.sample[5].smpdata[(int)smp_index];
			smp_index+=1/(((FREQ/note)/realLength));
			//smp_index++;
			if(smp_index >= realLength)
				smp_index -= realLength;
			//fprintf(stderr, "smp_index: %f\n", smp_index);
		}
	//}
}

int sdl_init(void){

	if(SDL_Init(SDL_INIT_AUDIO) < 0){
		printf("couldn't init SDL: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	requested.freq = FREQ;
	requested.format = AUDIO_S8;
	requested.samples = 512;
	requested.channels = 1;
	requested.callback = callback;

	SDL_OpenAudio(&requested, &obtained);

	fprintf(stderr, "freq %d\n", obtained.freq);
	if(obtained.format == 0x0008)
		fprintf(stderr, "format: AUDIO_U8\n");
	else if(obtained.format == 0x8008)
		fprintf(stderr, "format: AUDIO_S8\n");
	fprintf(stderr, "samples:%d\n", obtained.samples);
	fprintf(stderr, "channels:%d\n", obtained.channels);

	return 0;
}

int main(int argc, char **argv){
	FILE *modfile;
	FILE *samplefile;
	int i;
	int trash;
	u8 highestPattern = 0;

	const u16 octave1Period[12] = {
		856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453
	};

	u16 periodTable[12*5];
	u8 octave,note;
	char sig[4];

	for(octave = 0; octave < 5; octave++){
		for(note = 0; note < 12; note++){
			//*2 to get us into octave 0, then divide by 2 for each
			// octave from there
			periodTable[octave*12 + note] =
					(octave1Period[note]*2) >> octave;
		}
	}

	/* ~opening the modfile~ */
	modfile = fopen(argv[1], "rb");

	samplefile = fopen("samp.raw", "wb");

	if(!modfile){
		printf("Couldn't open file!\n");
		return 1;
	}

	/* ~is it a mod?~ */
	// probably unnecessary
	fseek(modfile, 1080, SEEK_SET);
	fread(&sig, 1, 4, modfile);
	if(!strcmp(sig,"M.K.")){
		printf("not a MOD\n");
		printf(sig);
		exit(1);
	}else{
		printf("YES");
	}

	/* ~what's its name?~ */
	rewind(modfile);
	fread(modheader.name, 20, 1, modfile);
	modheader.name[19] = '\0';

	/* ~ load samples ~*/
	for(i = 0; i < 31; i++){
		fread(&modheader.sample[i], 30, 1, modfile);
		modheader.sample[i].smpdata = NULL;

		modheader.sample[i].length =
			( ((modheader.sample[i].length & 0xff) << 8) |
				(modheader.sample[i].length >> 8) );

		modheader.sample[i].loopstart =
			( ((modheader.sample[i].loopstart & 0xff) << 8) |
				(modheader.sample[i].loopstart >> 8) );

		modheader.sample[i].looplength =
			( ((modheader.sample[i].looplength & 0xff) << 8) |
				(modheader.sample[i].looplength >> 8) );
	}

	/*~ load orders ~*/
	fread(&modheader.orderCount, 1, 1, modfile);

	fread(&trash, 1, 1, modfile); //useless byte

	fread(&modheader.order, 128, 1 , modfile);

	fread(&trash, 4, 1, modfile);

	/*~ load patterns ~*/
	for(i = 0; i < modheader.orderCount; i++){
		if(modheader.order[i] > highestPattern)
			highestPattern = modheader.order[i];
	}

	modheader.patternCount = highestPattern + 1;

	// allocate space for patterns
	modheader.patterns = malloc(sizeof(struct pattern[modheader.patternCount]));

	if(modheader.patterns==NULL){
		printf("out of memory!\n");
		exit(1);
	}


	//initialize modheader.pattern to 0
	/* XXX SEGFAULT */
	//memset(modheader.pattern, 0, modheader.patternCount*sizeof(u8*));

	u8 curPattern, row, column;

	for(curPattern = 0; curPattern < modheader.patternCount; curPattern++){
		//modheader.patterns[curPattern] = malloc(1024); //patterns are always 1k
		/*modheader.patterns[curPattern] = malloc(sizeof(struct pattern));
		if(!modheader.patterns[curPattern]){
			printf("out of memory!\n");
		}

		//initialize to 0
		memset(modheader.patterns[curPattern], 0, 1024);
		*/

		for(row = 0; row < 64; row++){
			for(column = 0; column < 4; column++){
				u8 cell[4];
				u8 sample;
				u16 period;
				u8 effect;
				u8 param;
				u8 closestNote;

				fread(cell, 4, 1, modfile);

				sample = (cell[0] & 0xf0) | (cell[2] >> 4);
				period = cell[1] | ((cell[0] & 0xf) << 8);
				effect = cell[2] & 0xf;
				param = cell[3];

				if(period == 0){
					closestNote = MOD_NO_NOTE; //period 0 is no note
				}

				else {
				u16 closestDist = 0xffff; //make sure the first comparison sets the closet note
				closestNote = 0;
				//looping through the period table
					for(i = 0; i < 12*5; i++){
						u16 newDist = abs(period - periodTable[i]);
						if(newDist < closestDist){
							closestNote = (u8)i;
							closestDist = newDist;
						}
					}
				}

				if(sample == 0){
					sample = MOD_NO_SAMPLE;
				}else {
					sample -= 1;
				}

				//now that we have our note, we can store the data in our new pattern
				//calculate the address of the cell to output to
				// rowoffset = row * 4 columns per row * 4 bytes per cell
				// columnoffset = column * 4 bytes per cell

				/*u8 *outCell = &modheader.pattern[curPattern][row*4*4 + column*4];
				outCell[0] = closestNote;
				outCell[1] = sample;
				outCell[2] = effect;
				outCell[3] = param;
				*/
				modheader.patterns[curPattern].pattern_entry[row][column].period = closestNote;
				modheader.patterns[curPattern].pattern_entry[row][column].sample = sample;
				modheader.patterns[curPattern].pattern_entry[row][column].effect = effect;
				modheader.patterns[curPattern].pattern_entry[row][column].param = param;
			}
		}
	}


	/* ~~ load sample datas ~~ */
	for(i = 0; i < 31; i++){
		int realLength = (modheader.sample[i].length) * 2;
		if(realLength != 0){
			modheader.sample[i].smpdata = malloc(realLength);
			if(modheader.sample[i].smpdata != NULL){
				fread(modheader.sample[i].smpdata, realLength, 1, modfile);
			}
		}
	}

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	//done reading modfile
	fclose(modfile);

	fwrite(modheader.sample[7].smpdata, (modheader.sample[7].length * 2), 1, samplefile);

	fclose(samplefile);

	printf("%s\n", modheader.name);

	for(i = 0; i < 31; i++)	{
		printf("%i: %s\n", i, modheader.sample[i].name);
		printf("%i: length:%i\n", i, modheader.sample[i].length);
	}

	printf("highest pattern: %i\n", highestPattern);

	printf("orderCount: %i\n",  modheader.orderCount);

	printf("patternCount: %i\n", modheader.patternCount);

	//printf("%i\n", sizeof(u8*[modheader.patternCount])); //36
	//printf("%i\n", sizeof(u8*)); //4

	/*for(i = 0; i < modheader.patternCount+1; i++)
		for(row = 0; row < 64; row++)
				printf("%i\n", modheader.pattern[i][row]);
				*/

	for(i = 0; i < modheader.orderCount; i++)
		printf("%i : %i\n", i, modheader.order[i]);

	for(row = 0; row < 64; row++){
		printf("%i %x %x %02x\n", modheader.patterns[0].pattern_entry[row][0].period,
			modheader.patterns[0].pattern_entry[row][0].sample + 1,
			modheader.patterns[0].pattern_entry[row][0].effect,
			modheader.patterns[0].pattern_entry[row][0].param);
	}

	if(sdl_init() == 0){
		SDL_PauseAudio(0);
		getchar();
		SDL_PauseAudio(1);
		SDL_Quit();
	}

	return 0;
}
