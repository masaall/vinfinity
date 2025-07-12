
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "x86.h"

struct cpu *mycpu(void){

	int32_t apicid, i;

	if (readrflags()&FL_IF)
		panic("mycpu called with interrupts enabled\n");

	apicid = lapicid();
	for (i = 0; i < ncpu; i++)
		if (cpus[i].apicid == apicid)
			return &cpus[i];	

	panic("unknown apicid\n");
}
