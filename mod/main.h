#define MOD_NO_NOTE	63
#define MOD_NO_SAMPLE	31

#define FREQ 44100

#define LOG(x, fmt)		fprintf(stdout, "LOG: %s : %" #fmt "\n", #x, x)
#define CHECK(x)		fprintf(stdout, "%s\n", #x)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

struct sample_header {
	u8 name[22]; //22 bytes
	u16 length; //2 bytes
	u8 finetune; //1 byte
	u8 vol; //1 byte
	u16 loopstart; //2 bytes
	u16 looplength; //2 bytes //more like loop-end
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
	int speed;
	int tempo;
};

struct mix_channel {
	char *data;
	double smp_index;
	double inc;
	u32 vol;
	u32 length;
	u32 looplength;

	int currsample;
	int currnote;

	int last_sample;
}mix_channels[4];

struct mod_header modheader;

void process_row(void);
s8 get_sample(struct mix_channel *chn);
s8 mix(void);
void update(void);
void init_player(void);

int sdl_init(void);
