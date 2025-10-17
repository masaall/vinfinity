
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "proc.h"
#include "irqs.h"

int sys_fork(void){
	return fork();
}

int sys_wait(void){
	return wait();
}

int sys_getpid(void){
	return myproc()->pid;
}

int sys_exit(void){
	exit();
	return 0;
}

int sys_sbrk(void){

	struct proc *p = myproc();
	uintptr_t size, n;

	n = p->regs->rdi;
	size = p->size;
	if (growproc(n) < 0)
		return -1;

	return size;
}
