
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "proc.h"

uintptr_t kernelstack;

void gdtinstall(void){
	gdt.pointer.limit = sizeof(gdt.entries) + sizeof(gdt.tss_ext) - 1;
	gdt.pointer.base = (uintptr_t)gdt.entries;

	asm volatile("lgdt %0" :: "m" (gdt.pointer));
}

void tssinstall(struct proc *p){

	uintptr_t addr = (uintptr_t)&gdt.tss;

	gdt.entries[5].limit_low = sizeof(gdt.tss);
	gdt.entries[5].base_low = addr & 0xffff;
	gdt.entries[5].base_middle = (addr >> 16) & 0xff;
	gdt.entries[5].base_high = (addr >> 24) & 0xff;
	gdt.tss_ext.base_highest = (addr >> 32) & 0xffffffff;
	gdt.tss.iomb = sizeof(gdt.tss);

	gdt.tss.rsp[0] = (uintptr_t)p->kstack + KSTACKSIZE;

	kernelstack = gdt.tss.rsp[0];

	asm volatile("mov $0x10,%%ax\n"
	    		 "mov %%ax,%%ds\n"
	    		 "mov %%ax,%%es\n"
	    		 "mov %%ax,%%ss\n"
	    		 "mov $0x2b,%%ax\n"
	    		 "ltr %%ax\n"
	    		 ::: "rax", "memory");
}
