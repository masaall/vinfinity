
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "irqs.h"
#include "x86.h"

#define IO_PIC1 0x20	// master (IRQs 0-7)
#define IO_PIC2	0xa0	// slave (IRQs 8-15)

struct idt idt[256];
extern uintptr_t vectors[];

void picinit(void){
	outb(IO_PIC1+1, 0xff);
	outb(IO_PIC2+1, 0xff);
}

void set_idt(uint16_t i, uintptr_t off){
	idt[i].off_low = off & 0xffff;
	idt[i].sel = 0x8;
	idt[i].type = 0x8e;
	idt[i].off_mid = (off >> 16) & 0xffff;
	idt[i].off_high = (off >> 32);
}

void idtinstall(void){

	uint16_t i;

	for (i = 0; i < 256; i++)
		set_idt(i, vectors[i]);

	struct {
		uint16_t size;
		uintptr_t base;
	} __attribute__((packed)) idtp = {
		sizeof(idt) - 1,
		(uintptr_t)idt
	};

	asm volatile("lidt %0" :: "m" (idtp));
}

void isr_handler(struct regs *r){

	switch (r->no){
	case T_IRQ0 + IRQ_TIMER:
		break;
	case T_IRQ0 + IRQ_KBD:
		kbdintr();
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_COM1:
		uartintr();
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_IDE:
		ideintr();
		lapiceoi();
		break;			
	default:
		if (myproc() == 0 || (r->cs&DPL_USER) == 0){
			cprintf("trap %d rip %p err %p rcr2 %p \n",
				r->no, r->rip, r->error, rcr2());
			cprintf("rsp %p cs %p ss %p \n", r->rsp, r->cs, r->ss);
			panic("trap");	
		}
		cprintf("pid %d: trap %d rip %p err %p rcr2 %p --kill proc\n",
			myproc()->pid, r->no, r->rip, r->error, rcr2());
		cprintf("rsp %p cs %p ss %p \n", r->rsp, r->cs, r->ss);	
		myproc()->killed = true;	
	}
	if (myproc() && myproc()->killed && (r->cs&DPL_USER) == DPL_USER)
		exit();
}
