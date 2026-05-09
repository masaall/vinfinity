
#include "types.h"
#include "defs.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "idt.h"

int sys_fork(struct registers*){
	return fork();
}

int sys_wait(struct registers*){
	return wait();
}

int sys_exit(struct registers*){
	exit();
	return 0;
}

int sys_getpid(struct registers*){
	return myproc()->pid;
}
