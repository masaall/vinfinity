
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "gdt.h"
#include "proc.h"
#include "mp.h"

struct cpu cpus[NCPU];
int ncpu;

uint8_t sum(char *addr, int n){

	int i, sum;

	sum = 0;
	for (i = 0; i < n; i++)
		sum += addr[i];

	return sum;
}

struct mp *mpsearch1(uintptr_t addr, uintptr_t size){

	char *start, *end;

	start = P2V(addr);
	end = start + size;
	for (; start < end; start += sizeof(struct mp)){
		if (memcmp(start,"_MP_",4) == 0 && sum(start,sizeof(struct mp)) == 0)
			return (struct mp*)start;
	}
	return 0;
}

struct mp *mpsearch(void){
	return mpsearch1(0xf0000, 0x10000);
}

struct mpconf *mpconfig(void){

	struct mp *mp;
	struct mpconf *conf;

	if ((mp = mpsearch()) == 0)
		return 0;
	conf = P2V(mp->physaddr);

	return conf;
}

void mpinit(void){

	struct mpconf *conf;
	struct mpproc *proc;
	char *p, *e;

	if ((conf = mpconfig()) == 0)
		panic("mpinit");
	lapic = P2V_DEV(conf->lapicaddr);
	p = (char*)(conf+1);
	e = (char*)conf+conf->length;
	for (; p < e;){
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
