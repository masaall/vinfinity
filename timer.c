
#include "types.h"
#include "defs.h"
#include "idt.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "x86.h"

uint64_t ticks;

void timer_handler(struct registers *){
	ticks++;

	lapiceoi();
	if (myproc() && myproc()->state == RUNNING)
		yield();
}

void timer_init(void){
	isr_install(T_IRQ0 + IRQ_TIMER, timer_handler);
}
