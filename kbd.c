
#include "types.h"
#include "defs.h"
#include "kbd.h"
#include "x86.h"

int kbdgetc(void){

	int stat, data, c;

	stat = inb(KBSTATP);

	if (!(stat & KBS_DIB)){
		return -1;
	}
	
	data = inb(KBDATAP);

	if (data & 0x80){
		return 0;
	}

	c = normalmap[data];

	return c;
}

void kbdintr(void){
	consintr(kbdgetc);
}
