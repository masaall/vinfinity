
#include "types.h"
#include "defs.h"
#include "idt.h"
#include "x86.h"
#include "msr.h"

extern void syscall_entry(void);

int sys_write(struct registers*);
int sys_fork(struct registers*);
int sys_wait(struct registers*);
int sys_exit(struct registers*);
int sys_getpid(struct registers*);
int sys_exec(struct registers*);

void syscall_init(void){
	wrmsr(MSR_EFER, (1<<0));
	wrmsr(MSR_STAR, (uint64_t)0x1b<<48|(uint64_t)0x8<<32);
	wrmsr(MSR_LSTAR, (uintptr_t)syscall_entry);
}

static int (*syscalls[])(struct registers*) = {
	[1] sys_write,
	[2] sys_fork,
	[3] sys_wait,
	[4] sys_exit,
	[5] sys_getpid,
	[6] sys_exec,
};

void syscall_handler(struct registers *regs){

	size_t num = regs->rax;

	if (num < sizeof(syscalls)/sizeof(syscalls[0])){
		regs->rax = syscalls[num](regs);
	} else {
		regs->rax = -1;
	}
}
