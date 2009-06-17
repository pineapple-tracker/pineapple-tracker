/* vi:set ts=8 sts=8 sw=8 noexpandtab: */

/*()_)()_)()_)()_)()_)()_)()_)()_)()_
 / the MUSICCHIP file format by LFT /
 ()_)()_)()_)()_)()_)()_)()_)()_)()_*/

#include <stdio.h>
#include <string.h>

#include "pineapple.h"
#include "gui.h"
#include "musicchip_file.h"

void savefile(char *fname){
	FILE *f;
	int i, j;

	f = fopen(fname, "w");
	if(!f){
		fprintf(stderr, "save error!\n");
		return;
	}

	fprintf(f, "musicchip tune\n");
	fprintf(f, "version 1\n");
	fprintf(f, "\n");
	fprintf(f, "%s\n", filename);
	fprintf(f, "\n");
	fprintf(f, "#%s\n", comment);
	fprintf(f, "\n");
	fprintf(f, "tempo: %d\n", callbacktime);
	for(i = 0; i < songlen; i++){
		fprintf(f, "songline %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i,
			song[i].track[0],
			song[i].transp[0],
			song[i].track[1],
			song[i].transp[1],
			song[i].track[2],
			song[i].transp[2],
			song[i].track[3],
			song[i].transp[3]);
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++){
		for(j = 0; j < tracklen; j++){
			struct trackline *tl = &track[i].line[j];

			if(tl->note || tl->instr || tl->cmd[0] || tl->cmd[1]){
				fprintf(f, "trackline %02x %02x %02x %02x %02x %02x %02x %02x\n",
					i,
					j,
					tl->note,
					tl->instr,
					tl->cmd[0],
					tl->param[0],
					tl->cmd[1],
					tl->param[1]);
			}
		}
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++){
		if(instrument[i].length > 1){
			for(j = 0; j < instrument[i].length; j++){
				fprintf(f, "instrumentline %02x %02x %02x %02x\n",
					i,
					j,
					instrument[i].line[j].cmd,
					instrument[i].line[j].param);
			}
		}
	}

	fclose(f);
}

int loadfile(char *fname){
	FILE *f;
	char buf[1024];
	int cmd[3];
	int i1, i2, trk[4], transp[4], param[3], note, instr;
	int i;
	char c;

	snprintf(filename, sizeof(filename), "%s", fname);

	f = fopen(fname, "r");
	if(!f){
		return -1;
	}

	songlen = 1;
	while(!feof(f) && fgets(buf, sizeof(buf), f)){
		if(1 == sscanf(buf, "#%1024c", &comment)){
		}
		else if(1 == sscanf(buf, "tempo: %hhd", &callbacktime)){
			callbacktime = (u8)callbacktime;
		}else if(9 == sscanf(buf, "songline %x %x %x %x %x %x %x %x %x",
			&i1,
			&trk[0],
			&transp[0],
			&trk[1],
			&transp[1],
			&trk[2],
			&transp[2],
			&trk[3],
			&transp[3])){

			for(i = 0; i < 4; i++){
				song[i1].track[i] = trk[i];
				song[i1].transp[i] = transp[i];
			}
			if(songlen <= i1) songlen = i1 + 1;
		}else if(8 == sscanf(buf, "trackline %x %x %x %x %x %x %x %x",
			&i1,
			&i2,
			&note,
			&instr,
			&cmd[0],
			&param[0],
			&cmd[1],
			&param[1])){

			track[i1].line[i2].note = note;
			track[i1].line[i2].instr = instr;
			for(i = 0; i < 2; i++){
				track[i1].line[i2].cmd[i] = cmd[i];
				track[i1].line[i2].param[i] = param[i];
			}
		}else if(4 == sscanf(buf, "instrumentline %x %x %x %x",
			&i1,
			&i2,
			&cmd[0],
			&param[0])){

			instrument[i1].line[i2].cmd = cmd[0];
			instrument[i1].line[i2].param = param[0];
			if(instrument[i1].length <= i2) instrument[i1].length = i2 + 1;
		}
	}

	fclose(f);
	return 0;
}

void saveinstrument(char *fname){
	FILE *f;
	int i;

	f = fopen(fname, "w");
	if(!f){
		fprintf(stderr, "save error!\n");
		return;
	}
	
	fprintf(f, "pineapple tune instrument\n");
	fprintf(f, "version alphamega\n");
	fprintf(f, "\n");
	fprintf(f, "%s\n", filename);
	fprintf(f, "\n");
	for(i = 0; i < instrument[currinstr].length; i++){
			fprintf(f, "instrumentline %02x %02x %02x\n",
				i,
				instrument[currinstr].line[i].cmd,
				instrument[currinstr].line[i].param);
	}
	fclose(f);
	return;
}

int loadinstrument(char *fname){
	FILE *f;
	char buf[1024];
	int i, cmd[3], param[3], instr;
	int fr;

	f = fopen(fname, "r");
	if(!f){
		return -1;
	}
	
	fr = _nextfreeinstr();

	while(!feof(f) && fgets(buf, sizeof(buf), f)){
		if(3 == sscanf(buf, "instrumentline %x %x %x",
			&i,
			&cmd[0],
			&param[0])){

			instrument[fr].line[i].cmd = cmd[0];
			instrument[fr].line[i].param = param[0];
			if(instrument[fr].length <= i) instrument[fr].length = i + 1;
		}
	}

	fclose(f);
	return 0;
}
