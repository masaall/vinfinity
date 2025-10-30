
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "x86.h"

extern char text[];
extern char data[];
extern char bss[];
extern char end[];

void mpmain(void);
void startothers(void);

uintptr_t entrypml4t[];
uintptr_t entrypdpt[];
uintptr_t entrypgdir[];

void screeninit(void){
	outb(0x3c6, 0xff);
	outb(0x3c8, 0);

	outb(0x3c9, 0x00);
	outb(0x3c9, 0x00);
	outb(0x3c9, 0x2a);
}

int main(void){

	screeninit();
	cls();
	uartinit();

	kinit1(end, P2V(0x400000));
	kvminit();
	mpinit();
	lapicinit();
	gdtinstall();
	picinit();
	ioapicinit();
	consoleinit();
	pinit();
	idtinstall();
	bufinit();
	fileinit();
	ideinit();
	cprintf("\n                         vinfinity operating system");
	cprintf("\n                                version 0.01     \n");	
	startothers();
	kinit2(P2V(0x400000), P2V(PHYSTOP));
	userinit();
	mpmain();

	panic("end main");
}

void mpmain(void){
	cprintf("cpu %d starting \n", mycpu()->apicid);
	idtinit();
	syscallinit();
	xchg((uintptr_t*)&(mycpu()->started), 1);
	scheduler();	
}

void mpenter(void){
	switchkvm();
	gdtinstall();
	lapicinit();
	mpmain();
}

void startothers(void){

	struct cpu *c;
	char *code, *stack;

	extern char _binary_entryother_start[], _binary_entryother_size[];

	code = P2V(0x7000);
	memmove(code, _binary_entryother_start, (uintptr_t)_binary_entryother_size);

	for (c = cpus; c < &cpus[ncpu]; c++){
		if (c == mycpu())
			continue;

	stack = kalloc();
	*(void**)(code-8) = stack + KSTACKSIZE;
	*(void**)(code-16) = mpenter;
	*(void**)(code-24) = (void*)V2P(entrypml4t);

	lapicstartap(c->apicid, V2P(code));

	while (c->started == 0);
	}
}

__attribute__((__aligned__(PGSIZE)))
uintptr_t entrypml4t[512] = {
	[0] = V2P(entrypdpt) + (PTE_P | PTE_W),
	[511] = V2P(entrypdpt) + (PTE_P | PTE_W),
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t entrypdpt[512] = {
	[0] = V2P(entrypgdir) + (PTE_P | PTE_W),
	[510] = V2P(entrypgdir) + (PTE_P | PTE_W),
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t entrypgdir[512] = {
	[0] = 0x00000000 + (PTE_P | PTE_W | PTE_PS),
	[1] = 0x00200000 + (PTE_P | PTE_W | PTE_PS),
};
