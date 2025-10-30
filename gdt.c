
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "x86.h"
#include "msr.h"

void set_gdt(struct cpu *c, int i, uint8_t access, uint8_t gran){
	c->gdt.entries[i].access = access;
	c->gdt.entries[i].gran = gran;
}

void gdtinstall(void){
	struct cpu *c = mycpu();

	set_gdt(c, 1, 0x9a, 0xa0);
	set_gdt(c, 2, 0x92, 0xa0);
	set_gdt(c, 3, 0xfa, 0xa0);
	set_gdt(c, 4, 0xf2, 0xa0);
	set_gdt(c, 5, 0xfa, 0xa0);

	c->gdt.pointer.size = sizeof(c->gdt.entries)+sizeof(c->gdt.tss_ext)-1;
	c->gdt.pointer.base = (uintptr_t)c->gdt.entries;
	asm volatile("lgdt %0" :: "m" (c->gdt.pointer));
}

void tssinstall(struct proc *p){

	struct cpu *c = mycpu();
	uintptr_t addr;

	addr = (uintptr_t)&c->gdt.tss;
	c->gdt.entries[6].limit = sizeof(c->gdt.tss) - 1;
	c->gdt.entries[6].base_low = addr & 0xffff;
	c->gdt.entries[6].base_middle = (addr >> 16) & 0xff;
	c->gdt.entries[6].access = 0x89;
	c->gdt.entries[6].base_high = (addr >> 24) & 0xff;
	c->gdt.tss_ext.base_highest = addr >> 32;
	
	c->gdt.tss.rsp[0] = (uintptr_t)p->kstack + KSTACKSIZE;
	wrmsr(KERNEL_GS_BASE, (uintptr_t)mycpu());
	mycpu()->kernelstack = (uintptr_t)p->kstack + KSTACKSIZE;

	asm volatile("ltr %0" :: "r" ((uint16_t)0x30));
}
