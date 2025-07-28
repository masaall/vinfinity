
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"

#include "proc.h"
#include "fs.h"

void mpmain(void) __attribute__((noreturn));

extern char text[];
extern char data[];
extern char bss[];
extern char end[];

void screen_init(void){
//	outb(0x3c6, 0xff);
//	outb(0x3c8, 0);

	outb(0x3c9, 0x00);
	outb(0x3c9, 0x00);
	outb(0x3c9, 0x2a);
}

int main(void){
	cls();
	screen_init();

	kinit1(end, P2V(0x400000));		
	kvmalloc();					
	mpinit();	
	gdtinstall();	
	picinit();							
	ioapicinit();				
	consoleinit();				
	pinit();
	tvinit();
	binit();		
	fileinit();
	userinit();
	mpmain();

	for (;;);
}

void mpmain(void){
	cprintf("\n                         vinfinity operating system");
	cprintf("\n                                version 1.0      \n");	
	idtinit();
	syscall_init();
	scheduler();
}
