
#include "types.h"
#include "defs.h"
#include "idt.h"
#include "x86.h"
#include "kbd.h"

int kbdgetc(void){

	uint32_t stat, data, c;

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

void keyboard_handler(struct registers*){
	consoleintr(kbdgetc);
	lapiceoi();
}

void keyboard_init(void){
	ioapicenable(IRQ_KBD, 0);
	isr_install(T_IRQ0+IRQ_KBD, keyboard_handler);
}
