
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "gdt.h"
#include "proc.h"
#include "mp.h"

struct cpu cpus[NCPU];
int ncpu;
uint8_t ioapicid;

uint8_t sum(uint8_t *addr, size_t n){

	size_t i, sum;

	sum = 0;
	for (i = 0; i < n; i++)
		sum += addr[i];

	return sum;
}

struct mp *mpsearch1(uintptr_t start, size_t size){

	uint8_t *addr, *end;

	addr = P2V(start);
	end = addr + size;
	for (; addr < end; addr += sizeof(struct mp))
		if (memcmp(addr, "_MP_", 4) == 0 
			&& sum(addr, sizeof(struct mp)) == 0)
				return (struct mp*)addr;
	
	return 0;
}

struct mp *mpsearch(void){
	return mpsearch1(0xf5000, 0x10000);
}

struct mpconf *mpconfig(void){

	struct mp *mp;
	struct mpconf *conf;

	if ((mp = mpsearch()) == 0)
		return 0;
	conf = P2V((uintptr_t)mp->physaddr);

	return conf;
}

void mpinit(void){

	struct mpconf *conf;
	struct mpproc *proc;
	struct mpioapic *ioapic;
	uint8_t *p, *e;

	if ((conf = mpconfig()) == 0)
		panic("mpinit");
	lapic = P2V_DEV((uintptr_t)conf->lapicaddr);
	p = (uint8_t*)(conf+1);
	e = (uint8_t*)conf+conf->length;
	while (p < e){
		switch (*p){
		case MPPROC:
			proc = (struct mpproc*)p;
			if (ncpu < NCPU){
				cpus[ncpu].apicid = proc->apicid;
				ncpu++;
			}
			p += sizeof(struct mpproc);
			continue;
		case MPIOAPIC:
			ioapic = (struct mpioapic*)p;
			ioapicid = ioapic->apicno;
			p += sizeof(struct mpioapic);
			continue;
		case MPBUS:
		case MPIOINTR:
		case MPLINTR:
			p += 8;
			continue;	
		}
	}
}
