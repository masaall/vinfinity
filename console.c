
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "va_list.h"

#define CRTPORT 0x3d4
#define BACKSPACE 0x100

extern char bss[];

static uint16_t *crt = P2V(0xb8000);

static void consputc(int);

void cls(void){

	uint16_t pos = 0;
	memset(crt, 0, sizeof(crt[0])*25*80);

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos>>8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
}

static void printint(long xx, int base, bool sign){

	static char digits[] = "0123456789abcdef";
	char buf[65];
	uint64_t x;
	int i = 0;

	if (sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);

	if (sign)
		buf[i++] = '-';

	while (--i >= 0)
		consputc(buf[i]);
}

void cprintf(const char *fmt, ...){

	int c;
	va_list va;

	va_start(va, fmt);
	for (size_t i = 0; (c = fmt[i] & 0xff); i++){
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
		case 'l':
			c = fmt[++i];
			switch (c){
			case 'd':
				printint(va_arg(va, long), 10, true);
				break;
			case 'x':
				printint(va_arg(va, uint64_t), 16, false);
				break;
			case 'b':
				printint(va_arg(va, uint64_t), 2, false);
				break;
			default:
				consputc('%');
				consputc('l');
				consputc(c);
				break;
			}
			break;
		case 'c':
			char ch = va_arg(va, int);
			consputc(ch);
			break;
		case 's':
			char *s = va_arg(va, char*);
			if (s == NULL) s = "null";
			for (; *s; s++)
				consputc(*s);
			break;
		default:
			consputc('%');
			consputc(c);
			break;
		}
	}
	va_end(va);
}

void getcallerpcs(uintptr_t *rbp, uintptr_t pcs[]){

	int i;

	for (i = 0; i < 10; i++){
		if (rbp == NULL || (char*)rbp < bss)
			break;

		pcs[i] = rbp[1];
		rbp = (void*)rbp[0];
	}

	for (; i < 10; i++)
		pcs[i] = 0;
}

void panic(const char *s){

	uintptr_t pcs[10];

	cprintf("panic: %s \n", s);
	getcallerpcs(__builtin_frame_address(0), pcs);
	for (size_t i = 0; i < 10; i++)
		cprintf("%p ", pcs[i]);
	cprintf("\n");
	for (;;);
}

static void cgaputc(int c){

	uint16_t pos;

	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1)<<8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	if (c == '\n'){
		pos += 80 - pos%80;
	} else if (c == BACKSPACE){
		if (pos > 0) pos--;
	} else
		crt[pos++] = 0x0f00 | c;

	if (pos >= 24*80){
		memmove(crt, crt+80, sizeof(crt[0])*23*80);
		pos -= 80;
		memset(crt+pos, 0, sizeof(crt[0])*(24*80-pos));
	}

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos>>8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
	crt[pos] = 0x0f00 | ' ';
}

void consputc(int c){
	cgaputc(c);
}

#define C(x) ((x) - '@')

void consoleintr(int (*getc)(void)){

	int c;
	bool is_show_proc = false;
	bool is_show_child = false;

	while ((c = getc()) >= 0){
		switch (c){
		case '-':
			is_show_child = true;
			break;
		case '=':
			is_show_proc = true;
			break;
		case C('H'):
			consputc(BACKSPACE);
			break;
		default:
			if (c != 0){
				consputc(c);				
			}
			break;
		}
	}

	if (is_show_proc){
		show_all_proc();
	}

	if (is_show_child){
		show_child_proc();
	}
}

int consolewrite1(const char *s, size_t n){
	for (size_t i = 0; i < n; i++){
		consputc(s[i] & 0xff);
	}

	return n;
}
