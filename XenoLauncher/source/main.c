/**
  * XenoLauncher, based on XenoShell
  *
  * Originally by cheqmate/adhs
  *
  * Continued by www.gc-forever.com members
  * emu_kidid, ...
  * 
  * XenoLauncher by drcref
  * https://github.com/drcref/xenogcfork
  */

#include "ulibsd/sd_io.h"
#include "ulibsd/spi_io.h"
#include "exi.h"
#include "pff/pff.h"

#define DEBUG 0

#define BS_ADDRESS 0x80800000

#define MEMCARD_A_EXI_REG_BASE	0xCC006800
#define MEMCARD_B_EXI_REG_BASE	0xCC006814
#define SP2_EXI_REG_BASE		0xCC006828

extern long GetMSR();
extern void SetMSR(long);
extern void dcache_flush_icache_inv(void *, int);

static void memset32(u32* pDest, u32 dwVal, u32 dwSize)
{
	while(dwSize--) *pDest++ = dwVal;
}

void LoadDolAtAddress(void *dol)
{
	struct dol_s 
	{
		u32 sec_pos[18];
		u32 sec_address[18];
		u32 sec_size[18];
		u32 bss_address, bss_size, entry_point;
	} *d = (struct dol_s*)dol;
	
	int i;
	for (i=0; i<18; ++i) 
	{
		if (!d->sec_size[i])
			continue;
		
		// copy section
		int nCount = d->sec_size[i];
		char *pDest = (char *) (void*)d->sec_address[i], *pSrc = (char *) ((u8*)dol)+d->sec_pos[i];
		while (nCount--)
			*pDest++ = *pSrc++;
		dcache_flush_icache_inv((void*)d->sec_address[i], d->sec_size[i]);
	}
	
	// clear BSS
	int nCount = d->bss_size;
	char *pDest = (char *) d->bss_address;
	while (nCount--)
		*pDest++ = 0;
		
	SetMSR((GetMSR() | 2) & ~0x8000);
	SetMSR(GetMSR() & ~0x8000); // EE
	SetMSR(GetMSR() | 0x2002);  // FP, RI
	
	void (*entrypoint)() = (void(*)())d->entry_point;
	entrypoint();
}

void abort(u32 type, u32 res) {
#if DEBUG
	ebase = MEMCARD_B_EXI_REG_BASE;
	exi_write_word(type);
	exi_write_word(res);
#endif
	while(1);
}

int main(void)
{
	memset32((void*)0x80000004, 0, (0x01700000-4)/4);
	*((u32*)0x80000C00) = 0x4c000064;

	FATFS fatfs;
	FRESULT res;
	u16 bytes_read;

	// attempt to mount SD card in Serial Port 2
	ebase = SP2_EXI_REG_BASE;
	res = pf_mount(&fatfs);
	if (res == FR_OK) goto load_file;

	// attempt to mount SD card in Memory Card Slot B
	ebase = MEMCARD_B_EXI_REG_BASE;
	res = pf_mount(&fatfs);
	if (res == FR_OK) goto load_file;
	
	// attempt to mount SD card in Memory Card Slot A
	ebase = MEMCARD_A_EXI_REG_BASE;
	res = pf_mount(&fatfs);
	if (res == FR_OK) goto load_file;

	// failed to mount SD card
	abort(1, res);

	load_file:

	// open file "swiss.dol" in root directory
	res = pf_open("SWISS   DOL\001");
	if (res != FR_OK) abort(2, res);

	// read DOL executable into memory
	res = pf_read((void*)BS_ADDRESS, fatfs.fsize, &bytes_read);
	if (res != FR_OK) abort(3, res);

	// delay to prevent swiss thrasing the disk drive
	GC_Sleep(2600);

	// load the DOL 
	LoadDolAtAddress((u8*)BS_ADDRESS);

	// something very wrong happened if we reach this point
	abort(4, res);

	return 0;
}
