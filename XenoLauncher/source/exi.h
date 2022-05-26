#ifndef _EXI_H_
#define _EXI_H_

#include "integer.h"

extern volatile u32* ebase;

/*** exi definitions ***/
// https://www.gc-forever.com/yagcd/chap5.html#sec5.9
#define EXI_SR		ebase[0]
#define EXI_DMA_MEM	ebase[1]
#define EXI_DMA_LEN	ebase[2]
#define EXI_CR		ebase[3]
#define EXI_DATA	ebase[4]
#define EXI_WAIT_EOT	while((EXI_CR)&1);

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

void exi_select(void);
void exi_deselect(void);
void exi_write_word(u32 word);
void exi_write_byte(u8 byte);
u32 exi_read_word();
u8 exi_read_byte();
void exi_read(u8 *dst, int len);
u32 tb_diff_msec(tb_t *end, tb_t *start);
void GC_Sleep(u32 dwMiliseconds);

#endif