
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
