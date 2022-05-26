/*
 *  File: integer.h
 *  Author: Nelson Lombardo
 *  Year: 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */
 
/*****************************************************************************/
/* Integer type definitions                                                  */
/*****************************************************************************/
#ifndef _INTEGER_H_
#define _INTEGER_H_


typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef volatile u8		vu8;
typedef volatile u16	vu16;
typedef volatile u32	vu32;


/* 16-bit, 32-bit or larger integer */
typedef long            INT;
typedef unsigned long   UINT;

/* 8-bit integer */
typedef char            CHAR;
typedef unsigned char   UCHAR;
typedef unsigned char   BYTE;
typedef unsigned char   BOOL;

/* 16-bit integer */
typedef short           SHORT;
typedef unsigned short  USHORT;
typedef unsigned short  WORD;
typedef unsigned short  WCHAR;

/* 32-bit integer */
typedef long            LONG;
typedef unsigned long   ULONG;
typedef unsigned long   DWORD;

/* Boolean type */
typedef enum { FALSE = 0, TRUE } BOOLEAN;
typedef enum { LOW = 0, HIGH } THROTTLE;

#endif

// «integer.h» is part of:
/*----------------------------------------------------------------------------/
/  ulibSD - Library for SD cards semantics            (C)Nelson Lombardo, 2015
/-----------------------------------------------------------------------------/
/ ulibSD library is a free software that opened under license policy of
/ following conditions.
/
/ Copyright (C) 2015, ChaN, all right reserved.
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/

// Derived from Mister Chan works on FatFs code (http://elm-chan.org/fsw/ff/00index_e.html):
/*----------------------------------------------------------------------------/
/  FatFs - FAT file system module  R0.11                 (C)ChaN, 2015
/-----------------------------------------------------------------------------/
/ FatFs module is a free software that opened under license policy of
/ following conditions.
/
/ Copyright (C) 2015, ChaN, all right reserved.
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/
