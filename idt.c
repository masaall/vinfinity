
#include "types.h"
#include "defs.h"
#include "idt.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "mmu.h"
#include "x86.h"

#define IO_PIC1 0x20
#define IO_PIC2 0xa0

#define NUM_ISRS 48

extern uintptr_t vectors[];

static void (*handlers[NUM_ISRS])(struct registers*);

static struct {
	struct gatedesc entries[256];
	struct idt_pointer pointer;
} idt;

void picinit(void){
	outb(IO_PIC1+1, 0xff);
	outb(IO_PIC2+1, 0xff);
}

void idt_set(size_t i, uintptr_t offset, uint8_t type){
	idt.entries[i] = (struct gatedesc) {
		.offset_low = offset,
		.selector = 0x8,
		.type = type,
		.offset_middle = offset >> 16,
		.offset_high = offset >> 32
	};
}

void idt_init(void){

	idt.pointer = (struct idt_pointer) {
		sizeof(idt.entries) - 1,
		(uintptr_t)idt.entries
	};

	asm volatile("lidt (%0)" :: "r" (&idt.pointer));
}

static const char *exceptions[32] = {
	"Divide Error",
	"Debug Exception",
	"NMI Interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack segment Fault",
	"General Protection",
	"Page Fault",
	"Reserved",
	"x87 FPU Floating-Point Error",
	"Alignmen Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception",
	"Control Protection Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

void isr_handler(struct registers *regs){
	if (handlers[regs->int_no]){
		handlers[regs->int_no](regs);
	}
}

void isr_install(size_t i, void (*handler)(struct registers*)){
	handlers[i] = handler;
}

void exception_handler(struct registers *regs){

	if (myproc() == NULL || (regs->cs&DPL_USER) == 0){
		cprintf("exception %d %s \n", regs->int_no ,exceptions[regs->int_no]);

		cprintf("rip %p err %p rcr2 %p\n", regs->rip, regs->err_no, rcr2());
		cprintf("rsp %p cs %p ss %p\n", regs->rsp, regs->cs, regs->ss);
		for (;;);
	}

	cprintf("exception %d %s \n",
	 regs->int_no ,exceptions[regs->int_no]);
	cprintf("pid %d: rip %p err %p rcr2 %p\n",
	myproc()->pid, regs->rip, regs->err_no, rcr2());
	cprintf("rsp %p cs %p ss %p\n", regs->rsp, regs->cs, regs->ss);

	exit();
}

void isr_init(void){
	for (size_t i = 0; i < 256; i++){
		idt_set(i, vectors[i], 0x8e);
	}

	for (size_t i = 0; i < 32; i++){
		isr_install(i, exception_handler);
	}
}
