/*
 *  File: spi_io.c.example
 *  Author: Nelson Lombardo
 *  Year: 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */

#include "spi_io.h"
#include "../exi.h"

/******************************************************************************
 Module Public Functions - Low level SPI control functions
******************************************************************************/

volatile tb_t start_time;
volatile u32 delay_ms;

void SPI_Init (void) {
    //EXI_SR = 0;
}

BYTE SPI_RW (BYTE d) {
    BYTE r;

    EXI_DATA = ((u32)d) << 24;
	EXI_CR = 0x09; // TLEN (data length) = 1 byte, RW (transfer type) = read and write
	EXI_WAIT_EOT;

    r = EXI_DATA >> 24;
    
    return r;
}
/*
BYTE SPI_R() {
    return exi_read_byte();
}

void SPI_W(BYTE d) {
    exi_write_byte(d);
}
*/
void SPI_Release (void) {
    SPI_CS_High();
}

inline void SPI_CS_Low (void) {
    // select device 1
	EXI_SR |= 0x80;
}

inline void SPI_CS_High (void){
    EXI_SR &= ~(0x80);
}

inline void SPI_Freq_High (void) {
    //EXI_SR |= 0x50; // 32 MHz
    //EXI_SR |= 0x40; // 16 MHz
    EXI_SR |= 0x30; // 8 MHz
}

inline void SPI_Freq_Low (void) {
    EXI_SR &= ~(0x30); // 1 MHz
}

void SPI_Timer_On (WORD ms) {
    mftb(&start_time);
    delay_ms = ms;
}

inline BOOL SPI_Timer_Status (void) {
    tb_t now;
    mftb(&now);
    if (tb_diff_msec(&now, &start_time) >= delay_ms)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

inline void SPI_Timer_Off (void) {

}

#ifdef SPI_DEBUG_OSC
inline void SPI_Debug_Init(void)
{
    
}
inline void SPI_Debug_Mark(void)
{
    
}
#endif

/*
The MIT License (MIT)

Copyright (c) 2015 Nelson Lombardo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
