
#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "mmu.h"
#include "memlayout.h"
#include "mp.h"
#include "proc.h"

struct cpu cpus[NCPU];
int ncpu;
uint8_t ioapicid;

uint8_t sum(uint8_t *addr, size_t len){

	uint32_t sum = 0;
	for (size_t i = 0; i < len; i++)
		sum += addr[i];

	return (uint8_t)(sum & 0xff);	
}

struct mp *mpsearch1(uint64_t a, size_t len){

	uint8_t *addr, *e, *p;

	addr = P2V(a);
	e = addr + len;
	for (p = addr; p < e; p += sizeof(struct mp))
		if (memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
			return (struct mp*)p;

	return 0;		
}

struct mp *mpsearch(void){
	return mpsearch1(0xf0000, 0x10000);
}

struct mpconf *mpconfig(){

	struct mp *mp;
	struct mpconf *conf;

	if ((mp = mpsearch()) == 0 || mp->physaddr == 0)
		return 0;
	conf = (struct mpconf*)P2V((uintptr_t)mp->physaddr);	
	if (memcmp(conf, "PCMP", 4) != 0)
		return 0;
	if (conf->version != 1 && conf->version != 4)
		return 0;	
	if (sum((uint8_t*)conf, conf->length) != 0)
		return 0;	
		
	return conf;
}

void mpinit(void){

	uint8_t *p, *e;
	int32_t ismp;
	struct mpconf *conf;
	struct mpproc *proc;
	struct mpioapic *ioapic;

	if ((conf = mpconfig()) == 0)
		panic("expect to run on an SMP");
	ismp = 1;	
	lapic = P2V_DEV((uintptr_t)conf->lapicaddr);
	for (p = (uint8_t*)(conf+1), e = (uint8_t*)conf+conf->length; p<e;){
		switch (*p){
			case MPPROC:
				proc = (struct mpproc*)p;
				if (ncpu < NCPU){
					cpus[ncpu].apicid = proc->apicid;
					cprintf("ncpu %d, apicid %d \n", ncpu, cpus[ncpu].apicid);
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
			default:
				ismp = 0;
				break;	
		}
	}

	if (!ismp)
		panic("didn't find a suitable machine");
}
