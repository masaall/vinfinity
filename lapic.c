
#include "types.h"
#include "memlayout.h"
#include "irqs.h"
#include "x86.h"

#define ID		(0x0020/4)	// ID
#define VER		(0x0030/4)	// version
#define TPR		(0x0080/4)	// task priority
#define EOI		(0x00b0/4)	// EOI
#define SVR		(0x00f0/4)	// spurious interrupt vector
#define ENABLE	0x00000100	// unit enable
#define ESR		(0x0280/4)	// error status
#define ICRLO	(0x0300/4)	// interrupt command
#define INIT	0x00000500	// INIT/RESET
#define STARTUP 0x00000600	// startup IPI
#define DELIVS	0x00001000	// delivery status
#define ASSERT	0x00004000	// assert interrupt (vs dassert)
#define DEASSERT 0x00000000
#define LEVEL	0x00008000	// level triggered
#define BCAST 	0x00080000	// send to all APICs, including self
#define BUSY	0x00001000	
#define FIXED	0x00000000
#define ICRHI	(0x0310/4)	// interrupt command [63:32]
#define TIMER	(0x0320/4)	// local vector table 0 (TIMER)
#define X1		0x0000000b	// divide counts by 1
#define PERIODIC 0x00020000	// periodic
#define PCINT	(0x0340/4)	// performance counter LVT
#define LINT0	(0x0350/4)	// local vector table 1 (LINT0)
#define LINT1	(0x0360/4)	// local vector table 2 (LINT1)
#define ERROR	(0x0370/4)	// local vector table 3 (ERROR)
#define MASKED	0x00010000	// interrupt masked
#define TICR	(0x0380/4)	// timer initial count
#define TCCR	(0x0390/4)	// timer current count
#define TDCR	(0x03e0/4)	// timer divide configuration
/*
volatile uint32_t *lapic;

void lapicw(int index, int value){
	lapic[index] = value;
}

void lapicinit(void){

	if (!lapic) return;

	lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
	lapicw(TICR, 10000000);
}

int lapicid(void){
	if (!lapic) return 0;

	return lapic[ID] >> 24;
}

void lapiceoi(void){
	if (lapic)
		lapicw(EOI, 0);
}

void lapicstartap(uint32_t apicid, uintptr_t addr){

	int i;

	lapicw(ICRHI, apicid<<24);
	lapicw(ICRLO, INIT | LEVEL | ASSERT);
	lapicw(ICRLO, INIT | LEVEL);

	for (i = 0; i < 2; i++){
		lapicw(ICRHI, apicid<<24);
		lapicw(ICRLO, STARTUP | (addr>>12));
	}
}
*/

volatile uint32_t *lapic;

static void lapicw(int index, int value){
	lapic[index] = value;
	lapic[ID];	// wait for write to finish, by reading
}

void lapicinit(void){
	if (!lapic)
		return;

	// enable local APIC; set spurious interrupt vector
	lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));	

	// the timer repeatedly counts down at bus frequency
	// from lapic[TICR] and then issues an interrupt
	lapicw(TDCR, X1);
	lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
	lapicw(TICR, 10000000);

	// disable logical interrupt lines
	lapicw(LINT0, MASKED);
	lapicw(LINT1, MASKED);

	// disable performance counter overflow interrupts
	// on machine that provide that interrupt entry
	if (((lapic[VER]>>16) & 0xff) >= 4)
		lapicw(PCINT, MASKED);

	// map error interrupt to IRQ_ERROR
	lapicw(ERROR, T_IRQ0 + IRQ_ERROR);

	// clear error status register (requires back-to-back writes)
	lapicw(ESR, 0);
	lapicw(ESR, 0);	

	// ack any outstanding interrupts
	lapicw(EOI, 0);

	// send an init level de-assert to synchronize arbitration ID's
	lapicw(ICRHI, 0);
	lapicw(ICRLO, BCAST | INIT | LEVEL);
	while (lapic[ICRLO] & DELIVS);

	// enable interrupts on the APIC (but not on the processor)
	lapicw(TPR, 0);
}


int lapicid(void){

	if (!lapic)
		return 0;
	return lapic[ID] >> 24;	
}

void lapiceoi(void){

	if (lapic)
		lapicw(EOI, 0);
}

void microdelay(int us){
	
}

#define CMOS_PORT	0x70
#define CMOS_RETURN 0x71

void lapicstartap(uint32_t apicid, uintptr_t addr){

	int i;
	uint16_t *wrv;

	// "the BSP must initialize CMOS shutdown code to 0AH
	// and the warn reset vector (DWORD based at 40:67) to point at
	// the AP startup code prior to the [universal startup algorithm]."	

	outb(CMOS_PORT, 0xf);	// offset 0xf is shutdown code
	outb(CMOS_PORT+1, 0x0A);
	wrv = (uint16_t*)P2V((0x40<<4 | 0x67));	// warn reset vector
	wrv[0] = 0;
	wrv[1] = addr >> 4;

	// "universal startup algorithm"
	// send INIT (level-triggered) interrupt to reset other CPU
	lapicw(ICRHI, apicid<<24);
	lapicw(ICRLO, INIT | LEVEL | ASSERT);
	microdelay(200);
	lapicw(ICRLO, INIT | LEVEL);
	microdelay(100);

	// send startup IPI (twice) to enter code
	// regular hardware is supposed to only accept a STARTUP
	// when it is in the halted state due to an INIT. So the second
	// should be ignored, but it is part of the official intel algorithm
	for (i = 0; i < 2; i++){
		lapicw(ICRHI, apicid<<24);
		lapicw(ICRLO, STARTUP | (addr>>12));
		microdelay(200);
	}
}
