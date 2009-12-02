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

void process_row(void);
