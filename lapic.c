
#include "types.h"
#include "idt.h"

#define ID 		(0x0020/4)	// ID
#define EOI		(0x00b0/4)	// EOI

#define TIMER 	(0x0320/4)	// local vector table 0 (TIMER)
#define X1		0x0000000b	// divide counts by 1
#define PERIODIC 0x00020000	// periodic
#define TICR	(0x0380/4)	// timer inital count
#define TDCR	(0x03e0/4)	// timer divide configuration

volatile uint32_t *lapic;

void lapicw(int index, int value){
	lapic[index] = value;
}

int lapicid(void){
	if (!lapic) return 0;

	return lapic[ID] >> 24;
}

void lapiceoi(void){
	if (!lapic)
		return;
	lapicw(EOI, 0);
}

void lapic_init(void){
	if (!lapic)
		return;

	lapicw(TDCR, X1);
	lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
	lapicw(TICR, 10000000);
}
