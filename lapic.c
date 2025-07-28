
#include "types.h"

#define ID 			(0x0020/4)		// ID
#define VER			(0x0030/4)		// version
#define TPR			(0x0080/4)		// task priority
#define EOI			(0x00b0/4)		// EOI

volatile uint32_t *lapic;

void lapicw(int32_t index, int32_t value){
	*(lapic + index) = value;
}

int32_t lapicid(void){

	if (!lapic)
		return 0;

	return *(lapic + ID) >> 24;	
}

void lapiceoi(void){
	if (lapic)
		lapicw(EOI, 0);
}
