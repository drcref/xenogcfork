#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

#define DEBUG 0

#define LOADER_ADDR 0x40D000

#define X_OUT 0x10 // PB4
#define X_OUT_PORT PORTB
#define X_OUT_PIN PINB
#define X_OUT_DDR DDRB
#define X_CLK 0x20 // PB5
#define X_CLK_PORT PORTB
#define X_CLK_PIN PINB
#define X_CLK_DDR DDRB
#define X_IN  0x08 // PB3
#define X_IN_PORT  PORTB
#define X_IN_PIN  PINB
#define X_IN_DDR  DDRB
#define X_STR 0x02 // PB1
#define X_STR_PORT PORTB
#define X_STR_PIN PINB
#define X_STR_DDR DDRB

// green
#define LED1_ON   PORTD &=~0x04
#define LED1_OFF  PORTD |= 0x04
// red
#define LED2_ON   PORTD &=~0x08
#define LED2_OFF  PORTD |= 0x08
#define LED_INIT  DDRD = 0x0C;


extern const u8 qcode[];
extern const u8* qcode_end;
extern const u8 uploader[];
extern const u8* uploader_end;
extern const u8 launcher[];
extern const u8* launcher_end;

void reset(void);
void ldelay(volatile int i);

#if DEBUG
// atmega328 specific UART registers
// modify for atmega8 if needed, check datasheet
void USART_Init( unsigned int baud )
{
	UCSR0A = 2;
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;
	/* Enable Receiver and Transmitter */
	UCSR0B = /* (1<<RXEN0)| */ (1<<TXEN0);
}


void USART_Transmit( unsigned char data )
{
	if (data == '\n')
		USART_Transmit('\r');
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

void putstring( char* data )
{
	while (*data) USART_Transmit(*data++);
}

// non-blocking single-char fast transmit
inline void putchar(char c)
{
	UDR0 = c;
}
#else
#define putstring(X)
#define putchar(X)
#endif // DEBUG

inline void delay(void)
{
	int i = 200;
	while (i--);
}

int ndelay = 0;

int io(char i)
{
	char res = 0;

	if (i)
		X_OUT_PORT |= X_OUT;
	else
		X_OUT_PORT &=~X_OUT;
	
	X_CLK_PORT &=~X_CLK;

	if (!ndelay) delay();

	res = X_IN_PIN & X_IN;

	X_CLK_PORT |= X_CLK;

	if (!ndelay) delay();
	
	return !!res;
}


// stock MN102 drivecode data format
// STR line is driven by MN102
// When MN102 is sending data, the STR line is normally-HIGH and it ends each byte with a ~2.875 us LOW strobe
// When MN102 is receiving data, the STR line is normally-LOW and it acknowledges each byte with a ~2.875 us HIGH strobe
// least significant bit first
// data is valid on CLK rising edge
// clock is HIGH when inactive

void send8(unsigned char c)
{
	int i = 0;
	
	while (X_STR_PIN & X_STR) {
		if (i++ > 10000) {
			putchar('f');
			reset();
		}
	}
	for (i=0; i<8; ++i)
	{
		if (X_STR_PIN & X_STR) {
			putchar('r');
			reset();
		}
		io(c & (1<<i));
	}
}


unsigned char recv8(void)
{
	unsigned char x = 0;
	int i=0;
	while (!(X_STR_PIN & X_STR)) {
		if (i++ > 10000) {
			putchar('w');
			reset();
		}
	}
	for (i=0; i<8; ++i)
	{
		if (!(X_STR_PIN & X_STR)) {
			putchar('s');
			reset();
		}
		x |= io(0) << i;
	}
	return x;
}

unsigned char io8(unsigned char c)
{
	unsigned char x = 0;
	int i;
	for (i=0; i<8; ++i)
		x |= io(c & (1<<i)) << i;
	return x;
}

// --------------------------------------------------------------
// SERIAL_CMD_FF:
// --------------------------------------------------------------
// Read memory
// MN102 first does a memcopy of the requested data
// to 0x40EB54 (max 32 bytes) then transmits it over serial

// R[0]	FF
// R[1]	00
// R[2]	source address middle byte
// R[3]	source address least significant byte
// R[4]	0
// R[5]	source address most significant byte
// R[6]	0
// R[7]	0
// R[8]	number of bytes to read
// R[9]	0

int read_mem(unsigned char *dst, long addr, int len)
{
	int err;

	send8(0xff);
	send8(0);
	send8(addr >> 8);
	send8(addr);
	send8(0);
	send8(addr >> 16);
	send8(0);
	send8(0);
	
	send8(len);
	send8(0);
	
	err = recv8();
	err |= recv8() << 8;
	
	if (err) {
		putchar('m');
		return err;
	}
	
	while (len--)
		*dst++ = recv8();

	return 0;
}

void write_word_norecv(long address, unsigned short data)
{
	send8(0xfe);
	send8(0x00);
	send8(address >> 8);
	send8(address);
	
	send8(data);
	send8(address >> 16);
	send8(data >> 8);
	send8(0);
	
	send8(2);
	send8(0);
}

// the firmware sends back 4 bytes when writing a word, discard them silently
void write_word(long address, unsigned short data)
{
	write_word_norecv(address, data);
	
	recv8();
	recv8();
	recv8();
	recv8();
}

void write_block(long address, unsigned char *source, int len)
{
	while (len >= 1) {
		// the firmware writes words in little endian order, swap endianness
		write_word(address, pgm_read_byte(source) | (pgm_read_byte(source+1) << 8));
		address += 2;
		len -= 2;
		source += 2;
	}
} 

void reset()
{
	// WDTCSR = 8; // atmega328p
	WDTCR = 8; // atmega8
	while (1);
}

void ldelay(volatile int i)
{
	while (i--);
}


int main(void)
{
	const unsigned int	qcode_size	= ((((const unsigned int) &qcode_end)	- ((const  unsigned int) &qcode))	& 0xFFFE) + 2;
	const unsigned int	uploader_size	= ((((const unsigned int) &uploader_end)	- ((const  unsigned int) &uploader))	& 0xFFFE) + 2;
	const unsigned int	launcher_size = ((((const unsigned int) &launcher_end)	- ((const  unsigned int) &launcher))	& 0xFFFE) + 2;

	/*
	// disable watchdog (for ATmega328P)
	MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = 0x00;
	*/

	LED_INIT;

	//USART_Init(3); // 250k baud

	u16 i;

	// X_CLK -> output
	// X_OUT -> output
	// X_STR -> input with internal pull-up
	// X_IN -> input without internal pull-up

	X_STR_PORT = X_STR;
	X_OUT_DDR = X_CLK | X_OUT;

	LED2_ON; LED1_OFF; 	// -> red

	u16 last_recv = 0;
	while (1) {
		last_recv >>= 1;
		last_recv |= io(1) ? 0x8000 : 0;
		if (last_recv == 0xeeee) {
			break;
		}
	}
	
	u16 launcher_flag = 0;
	read_mem(&launcher_flag, 0x40D100, 2);

	u8 request_launcher = (launcher_flag == 0x4444) ? 1 : 0;

	write_block(LOADER_ADDR, uploader, uploader_size);


	static const u8 PROGMEM stage1[] =	{
		0x80, 0x00,					//  8000		MOV	$00,D0				
		0xC4, 0xDA, 0xFC,			//  C4DAFC		MOVB	D0,($FCDA)	# disable breakpoints				
		0xF4,0x74,0x74,0x0a,0x08,	//	F47474A708  MOV	$080a74,a0		# restore original 
		0xF7,0x20,0x4C,0x80,		//	F7204C80    MOV	a0,($804c)		# inthandler
		0xF4,0x74,					//	F47400D040  MOV	QCODEIMGBASE,a0	# jump to drivecode init
		(LOADER_ADDR		& 0xFF),				
		(LOADER_ADDR >> 8	& 0xFF),
		(LOADER_ADDR >> 16	& 0xFF),		
		0xF0,0x00					//	F000        JMP	(a0)
	};

	// we write the above stage 1 into the MN102 at address 0x008674
	// then write the value 0x0086 at address 0x00804D
	// this executes the stage 1 loader
	// which in turn executes the stage 2 loader

	write_block(0x8674, stage1, sizeof(stage1));
	write_word_norecv(0x804d, 0x0086);

	putchar('4');

	// now our stage 2 upload.bin is running on the drive chip
	// and is receiving our data from here on

	// now send either the drive code or the XenoLauncher binary

	u8* binary;
	u16 binary_size ;

	if (request_launcher) {
		binary = launcher;
		binary_size = launcher_size;
	} else {
		binary = qcode;
		binary_size = qcode_size;
	}

	io8(0x00);

	ndelay = 1; // disable delay for faster upload, our 2nd stage can receive data fast
	
	ldelay(100);
	io8(0x00); // response to this should be Q (0x51)

	ldelay(100);
	io8((binary_size >> 9)&0xFF);  // response to this should be C (0x43)
	ldelay(100);
	io8((binary_size >> 1)&0xFF);  // response to this should be O (0x4F)
	ldelay(100);
	io8(0); // response to this should be D (0x44)
	
	LED2_OFF;

	putchar('5');

	u8 r;
	u8 n;
	u8 e = (binary_size >> 1) & 0xFF; // first byte that comes back

	for (i = 0; i < binary_size; i++) {
		ldelay(100);

		n = pgm_read_byte(binary + i);
		r = io8(n);

		if (r != e)	{
			putchar('x');
			reset();
		}

		e = n;
	}

	putchar('6');

	ldelay(100);
	
	io8(0x21); // send final confirmation byte (uploader will reset if this is != 0x21)
	io8(0);
	io8(0);

	LED1_ON;

	while (!(X_STR_PIN & X_STR));

	putchar('7');

	reset();

	return 0; // keep gcc happy
}


