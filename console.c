
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "x86.h"
#include "va_list.h"
#include "irqs.h"

#define CRTPORT 0x3d4
#define BACKSPACE 0x100

uint16_t *crt = P2V(0xb8000);
static int panicked = 0;

struct {
	struct spinlock lock;
	int locking;
} cons;

void consputc(int c);

void printint(long xx, int base, bool sign){

	char digits[] = "0123456789abcdef";
	char val[21];
	uint64_t x;
	int i;

	if (sign && (sign = xx < 0))
		x = -xx;
	else 
		x = xx;

	i = 0;
	do {
		val[i++] = digits[x % base];
	} while ((x/=base) != 0);

	if (sign)
		val[i++] = '-';

	while (--i >= 0)
		consputc(val[i]);
}

void cprintf(const char *fmt, ...){

	int i, c;
	va_list va;
	char *s;
	char ch;

	if (cons.locking)
		acquire(&cons.lock);

	va_start(va, fmt);
	for (i = 0; (c = fmt[i] & 0xff); i++){
		if (c != '%'){
			consputc(c);
			continue;
		}
		c = fmt[++i];
		switch (c){
		case 'd':
			printint(va_arg(va, int), 10, true);
			break;
		case 'x':
			printint(va_arg(va, uint32_t), 16, false);
			break;
		case 'p':
			printint(va_arg(va, uintptr_t), 16, false);
			break;
		case 'b':
			printint(va_arg(va, uint32_t), 2, false);
			break;
		case 's':
			if ((s = va_arg(va, char*)) == 0)
				s = "null string";
			for (; *s; s++)
				consputc(*s);	
			break;
		case 'c':
			ch = va_arg(va, int);
			consputc(ch);
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
	va_end(va);

	if (cons.locking)
		release(&cons.lock);
}

void panic(char *s){

	uintptr_t pcs[10];
	int i;

	cprintf("lapicid %d: panic: ", lapicid());
	cprintf(s);
	cprintf("\n");
	getcallerpcs(__builtin_frame_address(0), pcs);
	for (i = 0; i < 10; i++)
		cprintf(" %p", pcs[i]);
	cprintf("\n");
	panicked = 1;
	for (;;);
}

void cls(){

	uint16_t pos = 0;
	
	memset(crt, 0, sizeof(crt[0])*25*80);
	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos >> 8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
}

void cgaputc(int c){

	uint16_t pos;

	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	if (c == '\n')
		pos += 80 - pos%80;
	else if (c == BACKSPACE){
		if (pos > 0) pos--;
	}	
	else 
		crt[pos++] = 0x0a00 | (c & 0xff);

	if (pos >= 24*80){
		memmove(crt, crt+80, sizeof(crt[0])*23*80);
		pos -= 80;
		memset(crt + pos, 0, sizeof(crt[0])*(24*80 - pos));
	}	

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos >> 8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
	crt[pos] = 0x0a00 | (' ' & 0xff);
}

void consputc(int c){

	if (panicked){
		cli();
		for (;;);
	}

	if (c == BACKSPACE){
		uartputc('\b'); uartputc(' '); uartputc('\b');
	} else
		uartputc(c);

	cgaputc(c);
}

#define INPUT_BUF 128
struct {
	char buf[INPUT_BUF];
	uint32_t r;
	uint32_t w;
	uint32_t e;
} input;

#define C(x)	((x) - '@')

void consintr(int (*getc)(void)){

	int c;

	acquire(&cons.lock);
	while ((c = getc()) >= 0){
		switch (c){
		case C('H'):
			if (input.e != input.w){
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		default:
			if (c != 0 && input.e-input.r < INPUT_BUF){
				input.buf[input.e++ % INPUT_BUF] = c;
				consputc(c);
				if (c == '\n' || input.e-input.r == INPUT_BUF){
					input.w = input.e;
					wakeup(&input.r);
				}
			}
			break;
		}
	}

	release(&cons.lock);
}

int consolewrite(struct inode *ip, char *addr, int n){

	int i;

	iunlock(ip);
	acquire(&cons.lock);
	for (i = 0; i < n; i++)
		consputc(addr[i] & 0xff);
	release(&cons.lock);	
	ilock(ip);

	return n;	
}

void consoleinit(void){

	initlock(&cons.lock, "console");
	
	devsw[CONSOLE].write = consolewrite;

	cons.locking = 1;

	ioapicenable(IRQ_KBD, 0);
}
