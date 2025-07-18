
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"

#include "fs.h"

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
	
	cprintf("text %p \n", text);
	cprintf("data %p \n", data);
	cprintf("bss  %p \n", bss);
	cprintf("end  %p \n", end);
	cprintf("     %p \n", P2V(0x400000));
	cprintf("     %p \n", P2V(PHYSTOP));

	kinit1(end, P2V(0x400000));		// phys page allocator
	kvmalloc();						// kernel page table
	mpinit();						// detect other processor
	seginit();						// segment descriptors
	ioapicinit();					// interrupt controller
	consoleinit();					// console hardware

	binit();						// buffer cache

	uint32_t addr[NDIRECT+1], addr1[NDIRECT+1];
	cprintf("size %d size %d\n", sizeof(addr), sizeof(addr1));

	addr[0] = 13;
	addr1[0] = 12;
	memmove(addr1, addr, sizeof(addr));
	cprintf("addr %d addr1 %d \n", addr[0], addr1[0]);

//	panic("end");

//	kinit1(P2V(0x400000), P2V(PHYSTOP));

	for (;;);
}
