
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "x86.h"
#include "va_list.h"

static int32_t panicked = 0;

static void consputc(int32_t);

void printint(int64_t xx, int32_t base, int32_t sign){

	static char digits[] = "0123456789abcdef";
	char buf[21];
	int32_t i;
	uint64_t x;

	if (sign && (sign = xx < 0))
		x = -xx;
	else 
		x = xx;

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);

	if (sign)
		buf[i++] = '-';

	while (--i >= 0)	
		consputc(buf[i]);	
}

void cprintf(char *fmt, ...){

	int32_t i, c;
	char *s;
	va_list ap;

	if (fmt == 0)
		panic("null fmt");

	va_start(ap, fmt);
	for ( i = 0; (c = fmt[i] & 0xff) != 0; i++){
		if (c != '%'){
			consputc(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if (c == 0)	break;
		switch (c){
		case 'd':
			printint(va_arg(ap, int32_t), 10, 1);
			break;
		case 'x':
			printint(va_arg(ap, uint32_t), 16, 0);
			break;	
		case 'p':
			printint(va_arg(ap, uintptr_t), 16, 0);
			break;
		case 'b':
			printint(va_arg(ap, uint32_t), 2, 0);
			break;	
		case 'l':
			c = fmt[++i] & 0xff;
			if (c == 0) break;
			switch (c){
				case 'd':
					printint(va_arg(ap, int64_t), 10, 1);
					break;
				case 'x':
					printint(va_arg(ap, uint64_t), 16, 0);
					break;
				default:
					consputc('%');
					consputc('l');
					consputc(c);
					break;	
			}
			break;	
		case 's':
			if ((s = va_arg(ap, char*)) == 0)
				s = "null";
			for (; *s; s++)
				consputc(*s);
			break;	
		case '%':
			consputc('%');
			break;
		default:
			consputc('%');
			consputc(c);
			break;
		}	
	}		

	va_end(ap);
}

#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static uint16_t *crt = (uint16_t*)P2V(0xb8000);

void cls(void){

	int32_t pos = 0;

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos >> 8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);

	memset(crt, 0, sizeof(crt[0])*25*80);
}

void panic(char *s){

	int32_t i;
	uintptr_t pcs[10];

	uintptr_t rbp;
	asm volatile("mov %%rbp,%0" : "=r" (rbp) : );
	cprintf("panic: rbp %p \n", rbp);
	cprintf("panic: &s %p \n", &s);

	cli();
	cprintf("lapicid %d: panic: ", lapicid());
	cprintf(s);
	cprintf("\n");
	getcallerpcs(__builtin_frame_address(0), pcs);
	for (i = 0; i < 10; i++)
		cprintf(" %p", pcs[i]);
	panicked = 1;
	for (;;);
}

static void cgaputc(int32_t c){

	int16_t pos;

	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	if (c == '\n')
		pos += 80 - pos%80;
	else if (c == BACKSPACE){
		if (pos > 0) --pos;
	} else
		crt[pos++] = (c&0xff) | 0x1a00;
		
	if (pos < 0 || pos > 25*80)
		panic("pos under/overflow");

	if ((pos/80) >= 24){
		memmove(crt, crt+80, sizeof(crt[0])*23*80);
		pos -= 80;
		memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
	}

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos >> 8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
	crt[pos] = ' ' | 0x1a00;
}

void consputc(int32_t c){

	if (panicked){
		cli();
		for (;;);
	}

	cgaputc(c);
}

void consoleinit(void){
	
}
