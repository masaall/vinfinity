
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"

struct gatedesc idt[256];
extern uintptr_t vectors[];

void tvinit(void){
	int32_t i;

	for (i = 0; i < 256; i++)
		SETGATE(idt[i], 0, SEG_KCODE << 3, vectors[i], 0); 	
	SETGATE(idt[T_SYSCALL], 1, SEG_KCODE << 3, vectors[T_SYSCALL], DPL_USER);		
}

void idtinit(void){
	lidt(idt, sizeof(idt));
}

void trap(struct trapframe *tf){

	if ((tf->cs&3) == DPL_USER)
		tf->cs = 0x1b;

	if (tf->trapno == T_SYSCALL){
		cprintf("trap syscall\n");
		return;
	}
		
	switch (tf->trapno){
		case T_IRQ0 + IRQ_TIMER:
		lapiceoi();
		break;
		case T_IRQ0 + IRQ_IDE:
		lapiceoi();
		break;
		case T_IRQ0 + IRQ_KBD:
		kbdintr();
		lapiceoi();
		break;
		case T_IRQ0 + IRQ_COM1:
		lapiceoi();
		break;
		case T_IRQ0 + IRQ_SPURIOUS:
		lapiceoi();
		break;
		default:
			if (myproc() == 0 || (tf->cs&3) == 0){
				cprintf("unexpected trap %d, rip %p \n",
					tf->trapno, tf->rip);
				panic("trap");		
	}
	cprintf("pid %d %s: trap %d err %d "
		 "rip %p --kill proc\n",
		 myproc()->pid, myproc()->name, tf->trapno,
		 tf->error, tf->rip);
	myproc()->killed = 1;	
	}

	if (myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
		exit();

}
