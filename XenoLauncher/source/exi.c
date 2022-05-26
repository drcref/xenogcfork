#include "exi.h"

// set EXI base address
volatile u32* ebase = (u32*) 0xCC006800;

void exi_select(void)
{
	EXI_SR |= 0x150;
}

void exi_deselect(void)
{
	EXI_SR &= ~0x100;
}

void exi_write_word(u32 word)
{
	EXI_DATA = word;
	EXI_CR = 0x35; // TLEN (data length) = 4 bytes, RW (transfer type) = write
	EXI_WAIT_EOT;
}

void exi_write_byte(u8 byte)
{
    EXI_DATA = ((u32)byte) << 24;
	EXI_CR = 0x05; // TLEN (data length) = 1 byte, RW (transfer type) = write
	EXI_WAIT_EOT;
}

u32 exi_read_word()
{
    EXI_DATA = 0;
    EXI_CR = 0x31;
    EXI_WAIT_EOT;
    return EXI_DATA;
}

u8 exi_read_byte()
{
    EXI_DATA = 0;
    EXI_CR = 0x01;
    EXI_WAIT_EOT;
    return EXI_DATA;
}

void exi_read(u8 *dst, int len)
{
	while (len)
	{
		int l = len;
		if (l > 4)
			l = 4;
		EXI_DATA = 0;
		EXI_CR = 0x31;
		EXI_WAIT_EOT;
		*(u32*)dst = EXI_DATA;
		dst += 4;
		len -= l;
	}
}

u32 tb_diff_msec(tb_t *end, tb_t *start)
{
	u32 upper, lower;
	upper = end->u - start->u;
	if (start->l > end->l)
		upper--;
	lower = end->l - start->l;
	return ((upper*((u32)0x80000000/(TB_CLOCK/2000))) + (lower/(TB_CLOCK/1000)));
}

void GC_Sleep(u32 dwMiliseconds)
{
	tb_t start, end;
	mftb(&start);
	while (1)
	{
		mftb(&end);
		if (tb_diff_msec(&end, &start) >= dwMiliseconds)
			break;
	}
}