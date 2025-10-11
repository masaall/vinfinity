
#include "types.h"

#define ID		(0x0020/4)	// ID
#define EOI		(0x00b0/4)	// EOI

uint32_t *lapic;

void lapicw(int index, int value){
	lapic[index] = value;
}

int lapicid(void){
	if (!lapic) return 0;

	return lapic[ID] >> 24;
}

void lapiceoi(void){
	if (lapic)
		lapicw(EOI, 0);
}
