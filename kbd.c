
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "kbd.h"

int32_t kbdgetc(void){

	static uint32_t shift;
	uint32_t st, data, c;

	st = inb(KBSTATP);
	if ((st & KBS_DIB) == 0)
		return -1; 
	data = inb(KBDATAP);

	if (data == 0xe0){
		shift |= E0ESC;
//		return 0;
	} else if (data & 0x80){
		shift &= ~E0ESC;
		return 0;
	} else if (shift & E0ESC){
		data |= 0x80;
		shift &= ~E0ESC;
	}

	shift ^= togglecode[data];
	c = normalmap[data];
	if (shift & CAPSLOCK){
		if ('a' <= c && c <= 'z')
			c += 'A' - 'a';
		else if ('A' <= c && c <= 'Z')
			c += 'a' - 'A';	
	}

	return c;
}

void kbdintr(void){
	consoleintr(kbdgetc);
}
