/*----------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module  R0.03a
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2019, ChaN, all right reserved.
/
/ Petit FatFs module is an open source software. Redistribution and use of
/ Petit FatFs in source and binary forms, with or without modification, are
/ permitted provided that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/-----------------------------------------------------------------------------/
/ Jun 15,'09  R0.01a  First release.
/
/ Dec 14,'09  R0.02   Added multiple code page support.
/                     Added write funciton.
/                     Changed stream read mode interface.
/ Dec 07,'10  R0.02a  Added some configuration options.
/                     Fixed fails to open objects with DBCS character.

/ Jun 10,'14  R0.03   Separated out configuration options to pffconf.h.
/                     Added _USE_LCC option.
/                     Added _FS_FAT16 option.
/
/ Jan 30,'19  R0.03a  Supported stdint.h for C99 and later.
/                     Removed _WORD_ACCESS option.
/                     Changed prefix of configuration options, _ to PF_.
/                     Added some code pages.
/                     Removed some code pages actually not valid.
/----------------------------------------------------------------------------*/

#include "pff.h"		/* Petit FatFs configurations and declarations */
#include "diskio.h"		/* Declarations of low level disk I/O functions */



/*--------------------------------------------------------------------------

   Module Private Definitions

---------------------------------------------------------------------------*/


#if PF_DEFINED != 8088	/* Revision ID */
#error Wrong include file (pff.h).
#endif

#define ABORT(err)	{fs->flag = 0; return err;}



/*--------------------------------------------------------*/
/* DBCS code ranges and SBCS extend char conversion table */
/*--------------------------------------------------------*/



/* Character code support macros */

#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))

#define IsDBCS1(c)	0
#define IsDBCS2(c)	0


/* FatFs refers the members in the FAT structures with byte offset instead
/ of structure member because there are incompatibility of the packing option
/ between various compilers. */

#define BS_jmpBoot			0
#define BS_OEMName			3
#define BPB_BytsPerSec		11
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BS_55AA				510

#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54

#define BPB_FATSz32			36
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82

#define MBR_Table			446

#define	DIR_Name			0
#define	DIR_Attr			11
#define	DIR_NTres			12
#define	DIR_CrtTime			14
#define	DIR_CrtDate			16
#define	DIR_FstClusHI		20
#define	DIR_WrtTime			22
#define	DIR_WrtDate			24
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28




/*--------------------------------------------------------------------------

   Private Functions

---------------------------------------------------------------------------*/


static FATFS *FatFs;	/* Pointer to the file system object (logical drive) */


/*-----------------------------------------------------------------------*/
/* Load multi-byte word in the FAT structure                             */
/*-----------------------------------------------------------------------*/

static WORD ld_word (const BYTE* ptr)	/*	 Load a 2-byte little-endian word */
{
	WORD rv;

	rv = ptr[1];
	rv = rv << 8 | ptr[0];
	return rv;
}

static DWORD ld_dword (const BYTE* ptr)	/* Load a 4-byte little-endian word */
{
	DWORD rv;

	rv = ptr[3];
	rv = rv << 8 | ptr[2];
	rv = rv << 8 | ptr[1];
	rv = rv << 8 | ptr[0];
	return rv;
}



/*-----------------------------------------------------------------------*/
/* String functions                                                      */
/*-----------------------------------------------------------------------*/

/* Fill memory block */
static void mem_set (void* dst, int val, int cnt) {
	char *d = (char*)dst;
	while (cnt--) *d++ = (char)val;
}

/* Compare memory block */
static int mem_cmp (const void* dst, const void* src, int cnt) {
	const char *d = (const char *)dst, *s = (const char *)src;
	int r = 0;
	while (cnt-- && (r = *d++ - *s++) == 0) ;
	return r;
}



/*-----------------------------------------------------------------------*/
/* FAT access - Read value of a FAT entry                                */
/*-----------------------------------------------------------------------*/

static CLUST get_fat (	/* 1:IO error, Else:Cluster status */
	CLUST clst	/* Cluster# to get the link information */
)
{
	BYTE buf[4];
	FATFS *fs = FatFs;

	if (clst < 2 || clst >= fs->n_fatent) return 1;	/* Range check */

	//if (disk_readp(buf, fs->fatbase + clst / 128, ((UINT)clst % 128) * 4, 4)) return 1;
	disk_readp(buf, fs->fatbase + clst / 128, ((UINT)clst % 128) * 4, 4);
	
	return ld_dword(buf) & 0x0FFFFFFF;
}




/*-----------------------------------------------------------------------*/
/* Get sector# from cluster# / Get cluster field from directory entry    */
/*-----------------------------------------------------------------------*/

static DWORD clust2sect (	/* !=0: Sector number, 0: Failed - invalid cluster# */
	CLUST clst		/* Cluster# to be converted */
)
{
	FATFS *fs = FatFs;


	clst -= 2;
	if (clst >= (fs->n_fatent - 2)) return 0;		/* Invalid cluster# */
	return (DWORD)clst * fs->csize + fs->database;
}


static CLUST get_clust (
	BYTE* dir		/* Pointer to directory entry */
)
{
	FATFS *fs = FatFs;
	CLUST clst = 0;


	clst = ld_word(dir+DIR_FstClusHI);
	clst <<= 16;
	clst |= ld_word(dir+DIR_FstClusLO);

	return clst;
}


/*-----------------------------------------------------------------------*/
/* Directory handling - Rewind directory index                           */
/*-----------------------------------------------------------------------*/

static FRESULT dir_rewind (
	DIR *dj			/* Pointer to directory object */
)
{
	CLUST clst;
	FATFS *fs = FatFs;


	dj->index = 0;
	clst = dj->sclust;
	if (clst == 1 || clst >= fs->n_fatent) {	/* Check start cluster range */
		return FR_DISK_ERR;
	}
	if (!clst) {	/* Replace cluster# 0 with root cluster# */
		clst = (CLUST)fs->dirbase;
	}
	dj->clust = clst;						/* Current cluster */
	dj->sect = clust2sect(clst);	/* Current sector */

	return FR_OK;	/* Seek succeeded */
}




/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory index next                        */
/*-----------------------------------------------------------------------*/

static FRESULT dir_next (	/* FR_OK:Succeeded, FR_NO_FILE:End of table */
	DIR *dj			/* Pointer to directory object */
)
{
	CLUST clst;
	WORD i;
	FATFS *fs = FatFs;


	i = dj->index + 1;
	if (!i || !dj->sect) return FR_NO_FILE;	/* Report EOT when index has reached 65535 */

	if (!(i % 16)) {		/* Sector changed? */
		dj->sect++;			/* Next sector */

		if (dj->clust == 0) {	/* Static table */
			if (i >= fs->n_rootdir) return FR_NO_FILE;	/* Report EOT when end of table */
		}
		else {					/* Dynamic table */
			if (((i / 16) & (fs->csize - 1)) == 0) {	/* Cluster changed? */
				clst = get_fat(dj->clust);		/* Get next cluster */
				//if (clst <= 1) return FR_DISK_ERR;
				if (clst >= fs->n_fatent) return FR_NO_FILE;	/* Report EOT when it reached end of dynamic table */
				dj->clust = clst;				/* Initialize data for new cluster */
				dj->sect = clust2sect(clst);
			}
		}
	}

	dj->index = i;

	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/

static FRESULT dir_find (
	DIR *dj,		/* Pointer to the directory object linked to the file name */
	BYTE *dir		/* 32-byte working buffer */
)
{
	FRESULT res;
	BYTE c;


	res = dir_rewind(dj);			/* Rewind directory object */
	if (res != FR_OK) return res;

	do {
		//res = disk_readp(dir, dj->sect, (dj->index % 16) * 32, 32)	/* Read an entry */
		//	? FR_DISK_ERR : FR_OK;
		disk_readp(dir, dj->sect, (dj->index % 16) * 32, 32);
		//if (res != FR_OK) break;
		c = dir[DIR_Name];	/* First character */
		if (c == 0) { res = FR_NO_FILE; break; }	/* Reached to end of table */
		if (!(dir[DIR_Attr] & AM_VOL) && !mem_cmp(dir, dj->fn, 11)) break;	/* Is it a valid entry? */
		res = dir_next(dj);					/* Next entry */
	} while (res == FR_OK);

	return res;
}

/*--------------------------------------------------------------------------

   Public Functions

--------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Locical Drive                                         */
/*-----------------------------------------------------------------------*/

FRESULT pf_mount (
	FATFS *fs		/* Pointer to new file system object */
)
{
	BYTE fmt, buf[36];
	DWORD bsect, fsize, tsect, mclst;


	FatFs = 0;

	if (disk_initialize() & STA_NOINIT) {	/* Check if the drive is ready or not */
		return FR_NOT_READY;
	}

	/* Search FAT partition on the drive */
	bsect = 0;

	/* Check a partition listed in top of the partition table */
	disk_readp(buf, bsect, MBR_Table, 16);	/* 1st partition entry */
	
	bsect = ld_dword(&buf[8]);	/* Partition offset in LBA */

	disk_readp(buf, bsect, BS_FilSysType32, 2); /* Check FAT32 */
	if (ld_word(buf) != 0x4146) return FR_NO_FILESYSTEM;
	
	/* Initialize the file system object */
	disk_readp(buf, bsect, 13, sizeof (buf));

	fsize = ld_word(buf+BPB_FATSz16-13);				/* Number of sectors per FAT */
	if (!fsize) fsize = ld_dword(buf+BPB_FATSz32-13);

	fsize *= buf[BPB_NumFATs-13];						/* Number of sectors in FAT area */
	fs->fatbase = bsect + ld_word(buf+BPB_RsvdSecCnt-13); /* FAT start sector (lba) */
	fs->csize = buf[BPB_SecPerClus-13];					/* Number of sectors per cluster */
	fs->n_rootdir = ld_word(buf+BPB_RootEntCnt-13);		/* Nmuber of root directory entries */
	tsect = ld_word(buf+BPB_TotSec16-13);				/* Number of sectors on the file system */
	if (!tsect) tsect = ld_dword(buf+BPB_TotSec32-13);
	mclst = (tsect						/* Last cluster# + 1 */
		- ld_word(buf+BPB_RsvdSecCnt-13) - fsize - fs->n_rootdir / 16
		) / fs->csize + 2;
	fs->n_fatent = (CLUST)mclst;

	fs->fs_type = FS_FAT32;

	fs->dirbase = ld_dword(buf+(BPB_RootClus-13));	/* Root directory start cluster */
	fs->database = fs->fatbase + fsize + fs->n_rootdir / 16;	/* Data start sector (lba) */

	fs->flag = 0;
	FatFs = fs;

	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

FRESULT pf_open (
	const char *filename	/* Pointer to the file name in raw FAT32 format */
)
{
	FRESULT res;
	DIR dj;
	BYTE dir[32];
	FATFS *fs = FatFs;


	if (!fs) return FR_NOT_ENABLED;		/* Check file system */

	fs->flag = 0;
	dj.fn = (BYTE *) filename;

	res = dir_find(&dj, dir); /* Find the file */
	if (res != FR_OK) return res;		/* File not found */

	fs->org_clust = get_clust(dir);		/* File start cluster */
	fs->fsize = ld_dword(dir+DIR_FileSize);	/* File size */
	fs->fptr = 0;						/* File pointer */
	fs->flag = FA_OPENED;

	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

FRESULT pf_read (
	void* buff,		/* Pointer to the read buffer (NULL:Forward data to the stream)*/
	UINT btr,		/* Number of bytes to read */
	UINT* br		/* Pointer to number of bytes read */
)
{
	DRESULT dr;
	CLUST clst;
	DWORD sect, remain;
	UINT rcnt;
	BYTE cs, *rbuff = buff;
	FATFS *fs = FatFs;


	*br = 0;
	// if (!fs) return FR_NOT_ENABLED;		/* Check file system */
	// if (!(fs->flag & FA_OPENED)) return FR_NOT_OPENED;	/* Check if opened */

	remain = fs->fsize - fs->fptr;
	if (btr > remain) btr = (UINT)remain;			/* Truncate btr by remaining bytes */

	while (btr)	{									/* Repeat until all data transferred */
		if ((fs->fptr % 512) == 0) {				/* On the sector boundary? */
			cs = (BYTE)(fs->fptr / 512 & (fs->csize - 1));	/* Sector offset in the cluster */
			if (!cs) {								/* On the cluster boundary? */
				if (fs->fptr == 0) {				/* On the top of the file? */
					clst = fs->org_clust;
				} else {
					clst = get_fat(fs->curr_clust);
				}
				//if (clst <= 1) ABORT(FR_DISK_ERR);
				fs->curr_clust = clst;				/* Update current cluster */
			}
			sect = clust2sect(fs->curr_clust);		/* Get current sector */
			//if (!sect) ABORT(FR_DISK_ERR);
			fs->dsect = sect + cs;
		}
		rcnt = 512 - (UINT)fs->fptr % 512;			/* Get partial sector data from sector buffer */
		if (rcnt > btr) rcnt = btr;
		dr = disk_readp(rbuff, fs->dsect, (UINT)fs->fptr % 512, rcnt);
		//if (dr) ABORT(FR_DISK_ERR);
		fs->fptr += rcnt;							/* Advances file read pointer */
		btr -= rcnt; *br += rcnt;					/* Update read counter */
		if (rbuff) rbuff += rcnt;					/* Advances the data pointer if destination is memory */
	}

	return FR_OK;
}