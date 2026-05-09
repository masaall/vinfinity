
#include "types.h"
#include "user.h"
#include "va_list.h"

static void putc(char c){
	write(&c, 1);	
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
		putc(buf[i]);
}

void printf(const char *fmt, ...){

	int c;
	va_list va;

	va_start(va, fmt);
	for (size_t i = 0; (c = fmt[i] & 0xff); i++){
		if (c != '%'){
			putc(c);
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
				putc('%');
				putc('l');
				putc(c);
				break;
			}
			break;
		case 'c':
			char ch = va_arg(va, int);
			putc(ch);
			break;
		case 's':
			char *s = va_arg(va, char*);
			if (s == NULL) s = "null";
			for (; *s; s++)
				putc(*s);
			break;
		default:
			putc('%');
			putc(c);
			break;
		}
	}
	va_end(va);
}
