#include <stdio.h>

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
	unsigned char orderCount;
	unsigned char patternCount;
};

int main(int argc, char **argv){
	FILE *modfile;
	struct mod_header modheader;
	int i;

	modfile = fopen(argv[1], "rb");

	if(!modfile) {
		printf("Couldn't open file!\n");
		return 1;
	}

	fseek(modfile, 0, SEEK_SET);
	fread(modheader.name, 20, 1, modfile);
	modheader.name[19] = '\0';
	
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

	fread(&modheader.orderCount, 1, 1, modfile);

	unsigned char trash;
	fread(&trash, 1, 1, modfile);

	fread(&modheader.order, 128, 1 , modfile);

	unsigned char highestPattern = 0;

	for(i = 0; i < modheader.orderCount; i++) {
		if(modheader.order[i] > highestPattern)
			highestPattern = modheader.order[i];
	}
	
	modheader.patternCount = highestPattern + 1;

	fclose(modfile);
		
	printf("%s\n", modheader.name);

	for(i = 0; i < 31; i++)	{
		printf("%i: %s\n", i, modheader.sample[i].name);
		printf("%i: length:%i\n", i, modheader.sample[i].length);
	}

	//printf("%i\n", sizeof(struct sample_header));

	printf("highest pattern: %i\n", highestPattern);

	return 0;
}
