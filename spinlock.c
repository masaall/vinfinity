
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "x86.h"

void pushcli(void){

	int eflags;

	eflags = readrflags();
	cli();
	if (mycpu()->ncli == 0)
		mycpu()->intena = eflags & FL_IF;
	mycpu()->ncli += 1;
}

void popcli(void){

	if (readrflags()&FL_IF)
		panic("popcli");
	if (--mycpu()->ncli < 0)
		panic("popcli");
	if (mycpu()->ncli == 0 && mycpu()->intena)
		sti();		
}
