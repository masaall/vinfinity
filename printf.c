
#include "types.h"
#include "user.h"
#include "va_list.h"

void putc(int fd, char c){
	write(fd, &c, 1);
}

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
		putc(1, val[i]);
}

void printf(const char *fmt, ...){

	int i, c;
	va_list va;
	char *s;
	char ch;

	va_start(va, fmt);
	for (i = 0; (c = fmt[i] & 0xff); i++){
		if (c != '%'){
			putc(1, c);
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
				putc(1, *s);	
			break;
		case 'c':
			ch = va_arg(va, int);
			putc(1, ch);
			break;	
		}
	}
}

