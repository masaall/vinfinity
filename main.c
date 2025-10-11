
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

int main(void){

	cls();
	uartinit();

	kinit1(end, P2V(0x400000));
	kvminit();
	kinit2(P2V(0x400000), P2V(PHYSTOP));
	mpinit();
	gdtinstall();
	picinit();
	ioapicinit();
	consoleinit();
	idtinstall();
	syscallinit();
	bufinit();
	ideinit();
	userinit();
	mpmain();

	for (;;);
}

void mpmain(void){
	cprintf("\n                         vinfinity operating system");
	cprintf("\n                                version 0.01     \n");
	cprintf("cpu %d starting \n", mycpu()->apicid);
	scheduler();	
}

uintptr_t entrypml4t[];
uintptr_t entrypdpt[];
uintptr_t entrypdpt1[];
uintptr_t entrypgdir[];

__attribute__((__aligned__(PGSIZE)))
uintptr_t entrypml4t[512] = {
	[0]   = V2P(entrypdpt) + PTE_P + PTE_W,
	[511] = V2P(entrypdpt1) + PTE_P + PTE_W,
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t entrypdpt[512] = {
	[0]   = V2P(entrypgdir) + PTE_P + PTE_W,
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t entrypdpt1[512] = {
	[510]   = V2P(entrypgdir) + PTE_P + PTE_W, 
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t entrypgdir[512] = {
	[0] = 0x00000000 | PTE_P | PTE_W | PTE_PS,
	[1] = 0x00200000 | PTE_P | PTE_W | PTE_PS,
};
