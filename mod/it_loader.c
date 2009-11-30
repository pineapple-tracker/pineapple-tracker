#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint16_t short_byteswap(uint16_t in) {
	in = (((in >> 8) & 0xff) | ((in << 8) & 0xff));
	/*char tmp;
	tmp = in[2];
	in[2] = in[1];
	in[1] = tmp;
	*/
	return in;

}

struct it_header {
	char sig[4];
	char songname[26];
	//0x20
	uint16_t ord_num;
	uint16_t ins_num;
	uint16_t smp_num;
	uint16_t pat_num;
	uint16_t cwt;
	uint16_t cmwt;
	uint16_t flags;
	uint16_t special;
	//0x30
	uint8_t gv; //global volume
	uint8_t mv; //mix volume
	uint8_t is; //initial speed
	uint8_t it; //initial tempo
	uint8_t sep; //panning separation between channels
	uint8_t pwd; //pitch wheel depth for midi controllers
	uint16_t msg_length; //length of song message
	uint32_t msg_offset; //message offset
	uint32_t reserved; //???
	//0x40
	uint8_t chnl_pan[64];
	//0x80
	uint8_t chnl_vol[64];
	//0xc0
	uint8_t *orders; //orders, in order ;). length is it_header.ord_num
};

int main(int argc, char **argv){
	FILE *file;
	struct it_header it_header;

	if(argc < 2){
		printf("usage: slurp <.it file>");
		return 1;
	}

	file = fopen(argv[1], "rb");

	if(!file){
		printf("couldnt open file!\n");
		return 1;
	}

	fseek(file, 0, SEEK_SET);

	/* read in IMPM sig */
	fread(it_header.sig, 1, 4, file);

	for(int i = 0; i < 4; i++)
		printf("%c", it_header.sig[i]);
	printf("\n");

	/* read in songname */
	fread(it_header.songname, 1, 26, file);
	printf("%s\n", it_header.songname);

	/* read in PHiligt */

	/* read in OrdNum */
	fseek(file, 0x20, SEEK_SET);
	//swap bytes
	fread(&it_header.ord_num, 1, 2, file);
	printf("Ordnum: %04x\n", it_header.ord_num);
	//it_header.ord_num = short_byteswap(it_header.ord_num);
	//printf("Ordnum: %04x\n", it_header.ord_num);

	/*read in insnum*/
	fread(&it_header.ins_num, 1, 2, file);
	//it_header.ins_num = short_byteswap(it_header.ins_num);
	printf("insnum: %d\n", it_header.ins_num);

	/* read in sampnum */
	fread(&it_header.smp_num, 1, 2, file);
	//it_header.smp_num = short_byteswap(it_header.smp_num);
	printf("smpnum: %d\n", it_header.smp_num);

	/* read in patnum */
	fread(&it_header.pat_num, 1, 2, file);
	printf("patnum: %d\n", it_header.pat_num);

	/* read in created with tracker version */
	fread(&it_header.cwt, 1, 2, file);
	printf("created with tracker version: %04x\n", it_header.cwt);

	/* read in compatabile with tracker version greater than */
	fread(&it_header.cmwt, 1, 2, file);
	printf("compatibile with tracker version >: %04x\n", it_header.cmwt);

	/* read in flags */
	fread(&it_header.flags, 1, 2, file);

	/* read in special */
	fread(&it_header.special, 1, 2, file);

	/* read in global vol. */
	fread(&it_header.gv, 1, 1, file);
	printf("global vol:%d\n", it_header.gv);

	/* read in mix vol. */
	fread(&it_header.mv, 1, 1, file);
	printf("mix vol:%d\n", it_header.mv);

	/* read in initial tempo and speed */
	fread(&it_header.is, 1, 1, file);
	fread(&it_header.it, 1, 1, file);
	printf("initial speed: %d, initial tempo: %d\n", it_header.is, it_header.it);

	/* read in sep */
	fread(&it_header.sep, 1, 1, file);
	/* read in pwd*/
	fread(&it_header.pwd, 1, 1, file);
	/* read in msg length */
	fread(&it_header.msg_length, 1, 2, file);
	/* read in msg offset */
	fread(&it_header.msg_offset, 1, 4, file);
	/* read in 4 reserved bytes */
	fread(&it_header.reserved, 1, 4, file);

	/* read in channel pans, 0x40 */
	for(int i = 0; i < 64; i++)
		fread(&it_header.chnl_pan[i], 1, 1, file);
	printf("chnlpans:\n");
	/* read in channel volumes, 0x80 */
	for(int i = 0; i < 64; i++)
		fread(&it_header.chnl_vol[i], 1, 1, file);
	printf("chnlvols:\n");

	/* read in order of orders , 0xc0*/
	it_header.orders = malloc(sizeof(it_header.ord_num));
	fread(it_header.orders, 1, it_header.ord_num, file);
	for(int i = 0; i < it_header.ord_num; i++)
		printf("%02x | ", it_header.orders[i]);
	printf("\n");

	return 0;
}
