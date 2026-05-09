
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "x86.h"
#include "msr.h"

void gdt_set(struct cpu *c, size_t i, uint8_t access, uint8_t gran){
	c->gdt.entries[i] = (struct segdesc) {
		.access = access,
		.gran = gran
	};
}

void gdt_init(void){

	struct cpu *c = mycpu();

	gdt_set(c, 1, 0x9a, 0xa0);
	gdt_set(c, 2, 0x92, 0x00);
	gdt_set(c, 3, 0x9a, 0xa0);
	gdt_set(c, 4, 0xf2, 0x00);
	gdt_set(c, 5, 0xfa, 0xa0);

	c->gdt.pointer.size = sizeof(c->gdt.entries)+sizeof(c->gdt.tss_ext)-1;
	c->gdt.pointer.base = (uintptr_t)c->gdt.entries;

	asm volatile("lgdt (%0)" :: "r" (&c->gdt.pointer));
}

void tss_install(struct proc *p){

	struct cpu *c = mycpu();
	uintptr_t addr = (uintptr_t)&c->gdt.tss;

	c->gdt.entries[6] = (struct segdesc) {
		.limit = sizeof(c->gdt.tss),
		.base_low = addr,
		.base_middle = addr >> 16,
		.access = 0x89,
		.base_high = addr >> 24
	};
	c->gdt.tss_ext.base_highest = addr >> 32;

	wrmsr(KERNEL_GS_BASE, (uintptr_t)c);
	c->gdt.tss.rsp[0] = (size_t)p->kstack + KSTACKSIZE;
	c->kernelstack = (size_t)p->kstack + KSTACKSIZE;

	asm volatile("ltr %0" :: "r" ((uint16_t)0x30));
}
