
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef volatile u8		vu8;
typedef volatile u16	vu16;
typedef volatile u32	vu32;

#define BS_ADDRESS 0x80800000

/*** memory layout ***/
#define MEM_TEMP		0x81000000
#define MEM_WORK		0x81010000
#define MEM_FONT		0x81020000
#define MEM_FB			((0xC0F00000))	// 80080000
#define MEM_FB2			((MEM_FB + 0x500))

#define GC_INIT_BASE		 (0x80000020)
#define GC_INIT_BASE_PTR	(u32*)GC_INIT_BASE

/*** video stuff ***/
#define VI_BASE			(u32*) 0xCC002000
#define VI_BASE2		(u32*) 0xCC002040
#define MBLIST_Y		(3 * 24)

#define R_VIDEO_FRAMEBUFFER_1	*(unsigned long*)0xCC00201C
#define R_VIDEO_FRAMEBUFFER_2	*(unsigned long*)0xCC002024

#define YBORDEROFFSET		(640*2 * 32)
#define CLAMP(x,l,h)		((x > h) ? h : ((x < l) ? l : x))
#define TENT(a, b, c)		(0.25 * (a) + 0.5 * (b) + 0.25 * (c))
#define RGB2YCBR(r,g,b)		((u32)(((u8)CLAMP((0.257 * (float)r + 0.504 * (float)g + 0.098 * (float)b +  16.0 + 0.5) , 16, 235) << 24) | ((u8)CLAMP((-0.148 * (float)r - 0.291 * (float)g + 0.439 * (float)b + 128.0 + 0.5), 16, 240) << 16) | ((u8)(0.257 * (float)r + 0.504 * (float)g + 0.098 * (float)b +  16.0 + 0.5) << 8) | (u8)CLAMP((0.439 * (float)r - 0.368 * (float)g - 0.071 * (float)b + 128.0 + 0.5), 16, 240)))

/*** exi definitions ***/
#define EXI_SR		ebase[0]
#define EXI_DMA_MEM	ebase[1]
#define EXI_DMA_LEN	ebase[2]
#define EXI_CR		ebase[3]
#define EXI_DATA	ebase[4]
#define EXI_WAIT_EOT	while((EXI_CR)&1);    

/*** controller definitions ***/
#define PAD_LEFT	0x0001
#define PAD_RIGHT	0x0002
#define PAD_DOWN	0x0004
#define PAD_UP		0x0008
#define PAD_Z		0x0010
#define PAD_R		0x0020
#define PAD_L		0x0040
#define PAD_A		0x0100
#define PAD_B		0x0200
#define PAD_X		0x0400
#define PAD_Y		0x0800
#define PAD_START       0x1000
#define PAD_CLEFT	0x2000
#define PAD_CRIGHT	0x4000
#define PAD_CDOWN	0x8000
#define PAD_CUP		0x10000

/* time */
#define TB_CLOCK	40500000
#define mftb(rval) ({unsigned long u; do { \
	 asm volatile ("mftbu %0" : "=r" (u)); \
	 asm volatile ("mftb %0" : "=r" ((rval)->l)); \
	 asm volatile ("mftbu %0" : "=r" ((rval)->u)); \
	 } while(u != ((rval)->u)); })

typedef struct {
	unsigned long l, u;
} tb_t;

/*** memcard stuff ***/
#define MEMCARD_EXI_READ_SIZE		0x200
#define MEMCARD_BLOCK_SIZE			0x2000
#define DOLSIZE			(500 * 1024)	// 512kb DOL
#define DOLSEARCH_RANGE		0x1000000	// (16Mb Card MAX)

#define MEMCARD_A_EXI_REG_BASE 0xCC006800
#define MEMCARD_B_EXI_REG_BASE 0xCC006814

struct MemCardHeader_t
{
	u8 unknown[12];
	u8 ostimeValue[8];
	u8 unknown2[12];
	u16 zeropadding;
	u16 memcardSizeInBits;
	u16 encoding;
	u8 unused[468];
	u16 updateCounter;
	u16 checksum1;
	u16 checksum2;
	u8 unused2[7680]
} __attribute__((__packed__));

typedef struct MemCardHeader_t MemCardHeader;

struct DirectoryEntry_t
{
	u32 gamecode;
	u16 makercode;
	u8 unused;
	u8 animKey;
	char filename[32]; // Ptr to a 32 char string
	u32 timestamp;
	u32 imageDataOffset;
	u16 iconsgfxformat;
	u16 iconsanimspeed;
	u8 filePermission;
	u8 copyCounter;
	u16 firstBlockIndex;
	u16 fileLength;
	u16 unused2;
	char* commentsStrings; // Apparently points to 2 32 char strings that must fit in a single block 
} __attribute__((__packed__));

typedef struct DirectoryEntry_t DirectoryEntry;

struct Directory_t
{
	DirectoryEntry entries[128];
} __attribute__((__packed__));

typedef struct Directory_t Directory;

struct BlockMap_t
{
	u16 checksum1;
	u16 checksum2;
	u16 updateCounter;
	u16 numFreeBlocks;
	u16 lastAllocatedBlockindex;
	u16 blockMapArray[0x0ffd];
} __attribute__((__packed__));

typedef struct BlockMap_t BlockMap;

#define INVALID_BLOCK 0x0000
#define LAST_BLOCK 0xFFFF

#define DIRECTORY_MAX_SIZE 128

typedef struct
{
	MemCardHeader header;
	Directory directory;
	Directory backupDirectory;
	BlockMap blockMap;
	BlockMap backupBlockMap;
} MemCard;

#define XENO_DOL_NAME "xeno.dol"

/*** dvd stuff ***/
#define MEM32(_x)		*(u32*)_x		//---------------------------------------------------------------------------------------------------
#define DVD_DISR		MEM32( 0xcc006000 )	// DI Status Register
#define DVD_DISR_TCINT		(1 << 4)		// TCINT - Transfer Complete Interrupt Status
							// read : 0 Interrupt has not been requested / 1 Interrupt has been requested
							// write: write 0 no effect / 1 clear Interrupt
#define DVD_DISR_DEINT		(1 << 2)		// DEINT - Device Error Interrupt Status
							// read : 0 Interrupt has not been requested / 1 Interrupt has been requested
							// write: write 0 no effect / 1 clear Interrupt
#define DVD_DICR_DMA		(1 << 1)		// DMA - 0: immediate mode, 1: DMA mode (*1)
#define DVD_DICR_TSTART		(1 << 0)		// TSTART - transfer start. 
							// write 1: start transfer, read 1: transfer pendin
							//---------------------------------------------------------------------------------------------------
#define DVD_CLEARSTATUS()	DVD_DISR |= ( DVD_DISR_DEINT | DVD_DISR_TCINT );
#define DVD_WAIT_STATUS()	while( ! ( DVD_DISR & ( DVD_DISR_DEINT | DVD_DISR_TCINT)))

/*** function prototypes ***/
static void load_apploader();
/* unused
static void memcpy32(u32* pDest, u32* pSrc, u32 dwSize);
*/
static void memset32(u32* pDest, u32 dwVal, u32 dwSize);
void InitSystem( unsigned long VidMode );
