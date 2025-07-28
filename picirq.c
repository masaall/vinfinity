
#include "types.h"
#include "x86.h"

#define IO_PIC1		0x20	// master (IRQs 0 - 7)
#define IO_PIC2		0xa0	// slave (IRQs 8 - 15)

void picinit(void){
	outb(IO_PIC1+1, 0xff);
	outb(IO_PIC2+1, 0xff);
}
