
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "mp.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "param.h"

struct cpu cpus[NCPU];
int ncpu;

uint8_t sum(uint8_t *addr, size_t n){

	int sum = 0;
	for (size_t i = 0; i < n; i++)
		sum += addr[i];

	return sum;
}

struct mp *mpsearch1(uintptr_t paddr, size_t n){

	uint8_t *vaddr = P2V(paddr);
	uint8_t *vend = vaddr + n;
	for (; vaddr < vend; vaddr += sizeof(struct mp)){
		if (memcmp(vaddr, "_MP_",4) == 0 &&
		    sum(vaddr, sizeof(struct mp)) == 0)
			return (void*)vaddr;
	}

	return 0;
}

struct mp *mpsearch(void){
	return mpsearch1(0xf0000, 0x10000);
}

struct mpconf *mpconfig(void){
	struct mp *mp = mpsearch();
	struct mpconf *conf = P2V((uintptr_t)mp->physaddr);

	return conf;	
}

void mpinit(void){

	struct mpconf *conf = mpconfig();
	lapic = P2V_DEV((uintptr_t)conf->lapicaddr);

	uint8_t *p, *e;
	p = (void*)(conf + 1);
	e = (uint8_t*)conf + conf->length;

	for (; p < e;){
		switch (*p){
		case MPPROC:
			struct mpproc *proc = (void*)p;
			if (ncpu < 8){
				cpus[ncpu].apicid = proc->apicid;
				ncpu++;
			}
			p += sizeof(struct mpproc);
			continue;
		case MPIOAPIC:
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
