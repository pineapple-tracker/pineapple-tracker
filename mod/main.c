#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <SDL/SDL.h>

#include "main.h"

double smp_index = 0;
double note = 65.41; //c3

int currpatt = 0;
int currrow = 0;
int currorder = 0;
int currsample;
int currnote;
int tick = 6;
int last_sample;
int samples_per_tick;
int samples_left; //counter

SDL_AudioSpec requested, obtained;

//static char *notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-",
//				"G#", "A-", "A#", "H-"};

/*double note_table[] = {
	//16.35, 17.32, 18.35, 19.45, 20.6, 21.83, 23.12, 24.5, 25.96, 27.5, 29.14, 30.87, //C-1..B-1
	32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25, 49.00, 51.91, 55, 58.27, 61.74, //C-2..B-2
	65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.5, 98, 103.83, 110, 116.54, 123.47, //C-3..B-3
	130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185, 196, 207.65, 220, 233.08, 246.94, //C-4..B-4
	261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392, 415.3, 440, 466.16, 493.88 //C-5..B-5
};
*/
/*smp_index+=(
pow(2.0, (currnote-24.0)/12.0)*1.0)
/((float)FREQ/8363.0);*/
const double inc_table[60] = {
	0.250000, 0.264866, 0.280616, 0.297302, 0.314980, 0.333710, 0.353553, 0.374577, 0.396850, 0.420448, 0.445449, 0.471937,
	0.500000, 0.529732, 0.561231, 0.594604, 0.629961, 0.667420, 0.707107, 0.749154, 0.793701, 0.840896, 0.890899, 0.943874,
	1.000000, 1.059463, 1.122462, 1.189207, 1.259921, 1.334840, 1.414214, 1.498307, 1.587401, 1.681793, 1.781797, 1.887749,
	2.000000, 2.118926, 2.244924, 2.378414, 2.519842, 2.669680, 2.828427, 2.996614, 3.174802, 3.363586, 3.563595, 3.775497,
	4.000000, 4.237852, 4.489848, 4.756828, 5.039684, 5.339359, 5.656854, 5.993228, 6.349604, 6.727171, 7.127190, 7.550995,
};

void init_player(void){
	modheader.speed = 3;
	modheader.tempo = 150;
	samples_per_tick = FREQ / (2 * modheader.tempo / 5);
	samples_left = samples_per_tick;
	tick = 0;
	currpatt = modheader.order[0];
	currrow = 0;
	currorder = 0;
}

s8 get_sample(struct mix_channel *chn){
	//LOG(chn->currsample, i);
	if(chn->currnote == 0)
		return 0;
	chn->smp_index+=inc_table[chn->currnote]/((float)FREQ/8363.0);
	if(modheader.sample[chn->currsample].length > 2){
		if(chn->smp_index >= (modheader.sample[chn->currsample].looplength + modheader.sample[chn->currsample].loopstart))
			chn->smp_index = modheader.sample[chn->currsample].loopstart;
	}else{
		if(chn->smp_index >= modheader.sample[chn->currsample].length){
			chn->currnote = 0;
			chn->smp_index = 0;
		}
	}
	if(obtained.format == AUDIO_U8)
		return modheader.sample[chn->currsample].smpdata[(int)chn->smp_index] + 128;
	else
		return modheader.sample[chn->currsample].smpdata[(int)chn->smp_index];
}

/* the worst mixer in the wurld */
s8 mix(void){
	s16 temp_buf;
	//only four channels for now...
	for(int i = 0; i < 4; i++){
		temp_buf += get_sample(&mix_channels[i]);
	}
	// >>6 to divide off the volume, >>2 to divide by 4 channels to prevent overflow
	return temp_buf >> 8;
}

void process_tick(void){
	if(tick >= modheader.speed){
		tick = 0;
		process_row();
	}else
		tick++;
}

void process_row(void){
	//iterate through channels
	for(int i = 0; i < 4; i++){
		if(modheader.patterns[currpatt].pattern_entry[currrow][i].sample == MOD_NO_SAMPLE)
			mix_channels[i].currsample = mix_channels[i].last_sample;
		else{
			mix_channels[i].last_sample = modheader.patterns[currpatt].pattern_entry[currrow][i].sample;
			mix_channels[i].currsample = mix_channels[i].last_sample;
			mix_channels[i].smp_index = 0;
		}
		if(modheader.patterns[currpatt].pattern_entry[currrow][i].period != MOD_NO_NOTE)
			mix_channels[i].currnote = modheader.patterns[currpatt].pattern_entry[currrow][i].period;
	}

	if(currrow++ >= 64){
		currrow = 0;
		if(currorder++ >= modheader.orderCount)
			currorder = 0;
		currpatt = modheader.order[currorder];
	}
}

void callback(void *data, Uint8 *buf, int len){
	int pos = 0;
	int buffer_left = len;
	s8 *out;
	out = (s8*) buf;
	while(buffer_left > 0){
		//LOG(buffer_left, i);
		if(samples_left <= 0){
			//CHECK(1);
			process_tick();
			samples_left = samples_per_tick;
			//CHECK(ifstatement);
			//LOG(buffer_left, i);
			//LOG(samples_left, i);
		}
		while((buffer_left > 0) && (samples_left > 0)){
			/* probably should wrap this up into its own function... */
			/* sometimes you get unsigned audio even if you ask for signed :| */
			out[pos] += mix();
			//out[pos] = get_sample(&mix_channels[0]);
			pos += 1;
			buffer_left -= 1;
			samples_left -= 1;
			//CHECK(inner-loop);
			//LOG(buffer_left, i);
			//LOG(samples_left, i);
		}
	}
	//CHECK(post-loop);
	//LOG(buffer_left, i);
	//LOG(samples_left, i);
}

int sdl_init(void){

	if(SDL_Init(SDL_INIT_AUDIO) < 0){
		printf("couldn't init SDL: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	requested.freq = FREQ;
	requested.format = AUDIO_S8;
	requested.samples = 1024;
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

	/*const u16 period_table[60] = {
		1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 906,
		856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
		428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
		214, 202, 190, 180, 169, 160, 151, 142, 134, 127, 120, 113,
		107, 101, 95, 90, 84, 80, 75, 71, 67, 63, 60, 56
	};*/

	u16 periodTable[12*5];
	u8 octave,note;
	char sig[4];

	for(octave = 0; octave < 5; octave++){ //12 is c-3
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
	fseek(modfile, 1080, SEEK_SET);
	fread(&sig, 1, 4, modfile);
	if(!strcmp(sig,"M.K.")){
		printf("not a MOD\n");
		printf(sig);
		exit(1);
	}else{
		printf("YES");
	}
	/* need to check for other sigs later... */

	/* ~what's its name?~ */
	rewind(modfile);
	fread(modheader.name, 20, 1, modfile);
	modheader.name[19] = '\0';

	/* ~ load samples ~*/
	for(i = 0; i < 31; i++){
		fread(&modheader.sample[i], 30, 1, modfile);
		modheader.sample[i].smpdata = NULL;
		/* these bit shifty hacks only work because length, loopstart and
		looplength are u16 */
		modheader.sample[i].length =
			( ((modheader.sample[i].length & 0xff) << 8) |
				(modheader.sample[i].length >> 8) ) * 2; /*just times it by two when you
														  *load it the FIRST time.
													      *most samples will fit in a u16
														  *when multiplied by two but some
														  *probably won't!*/
		modheader.sample[i].loopstart =
			( ((modheader.sample[i].loopstart & 0xff) << 8) |
				(modheader.sample[i].loopstart >> 8) ) * 2;

		modheader.sample[i].looplength =
			( ((modheader.sample[i].looplength & 0xff) << 8) |
				(modheader.sample[i].looplength >> 8) ) * 2;
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
		int realLength = (modheader.sample[i].length);
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

	fwrite(modheader.sample[30].smpdata, (modheader.sample[30].length), 1, samplefile);

	fclose(samplefile);

	printf("%s\n", modheader.name);

	for(i = 0; i < 31; i++)	{
		printf("%0x: %s\n", i, modheader.sample[i].name);
		printf("%0x: length:%i\n", i, modheader.sample[i].length);
		printf("%0x: loopstart:%i\n", i, modheader.sample[i].loopstart);
		printf("%0x: looplength:%i\n", i, modheader.sample[i].looplength);
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
		init_player();
		SDL_PauseAudio(0);
		//while(1){
		//	update();
		//	usleep(5000);
		//}
		getchar();
		SDL_PauseAudio(1);
		SDL_Quit();
	}

	return 0;
}
