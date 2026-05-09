
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"

extern char text[];
extern char data[];
extern char bss[];
extern char end[];

void mem(void);

void screeninit(void){
	outb(0x3c9, 0x3f);
	outb(0x3c9, 0x0f);
	outb(0x3c9, 0x3f);
}

int main(void){

	screeninit();
	cls();

//	mem();

	kinit1((void*)PGUP(end), P2V(0x400000));

	kvminit();

	kinit1(P2V(0x400000), P2V(0xa000000));
	minit1(P2V(0xa000000), P2V(PHYSTOP));

	mpinit();
	lapic_init();
	gdt_init();
	picinit();
	ioapicinit();
	keyboard_init();
	timer_init();
	isr_init();
	idt_init();
	syscall_init();
	binit();
	ideinit();
	proc_init();
	init_proc();

	scheduler();

	for (;;);
}

void mem(void){
	cprintf("text %p \n", text);
	cprintf("data %p \n", data);
	cprintf("bss %p \n", bss);
	cprintf("end %p \n", end);
}

__attribute__((__aligned__(PGSIZE)))
uintptr_t pgdir[512] = {
	[0] = 0x00000000 | PTE_P | PTE_W | PTE_PS,
	[1] = 0x00200000 | PTE_P | PTE_W | PTE_PS,
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t pdpt[512] = {
	[0] = V2P(pgdir) + PTE_P + PTE_W,
	[510] = V2P(pgdir) + PTE_P + PTE_W,
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t pml4t[512] = {
	[0] = V2P(pdpt) + PTE_P + PTE_W,
	[511] = V2P(pdpt) + PTE_P + PTE_W,
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t pml5t[512] = {
	[0] = V2P(pml4t) + PTE_P + PTE_W,
	[511] = V2P(pml4t) + PTE_P + PTE_W,
};

