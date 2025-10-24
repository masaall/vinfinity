
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "irqs.h"
#include "msr.h"
#include "x86.h"
#include "syscall.h"

extern char end[];
extern void syscall_entry();

extern int sys_fork(void);
extern int sys_write(void);
extern int sys_wait(void);
extern int sys_getpid(void);
extern int sys_exit(void);
extern int sys_exec(void);
extern int sys_sbrk(void);
extern int sys_open(void);
extern int sys_mknod(void);
extern int sys_dup(void);

void syscallinit(void){
	wrmsr(MSR_EFER, 1 << 0);
	wrmsr(MSR_LSTAR, (uintptr_t)syscall_entry);
	wrmsr(MSR_STAR, (uintptr_t)0x1b<<48|(uintptr_t)0x8<<32);
//	wrmsr(KERNEL_GS_BASE, (uintptr_t)mycpu());
}

static int (*syscalls[])(void) = {
	[SYS_exec]	sys_exec,
	[SYS_fork] 	sys_fork,
	[SYS_write] sys_write,
	[SYS_wait]	sys_wait,
	[SYS_getpid] sys_getpid,
	[SYS_exit] sys_exit,
	[SYS_sbrk] sys_sbrk,
	[SYS_open] sys_open,
	[SYS_mknod] sys_mknod,
	[SYS_dup] sys_dup,
};

void syscall_handler(struct regs *r){

	int num;

	num = r->rax;
	if (num && syscalls[num] < (int(*)(void))end){
		r->rax = syscalls[num]();
	} else {
		cprintf("unknown syscall\n");
		r->rax = -1;
	}
}
