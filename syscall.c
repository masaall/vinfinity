
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "memlayout.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

#define MSR_EFER		 0xc0000080	// extended feature enable register
#define MSR_STAR		 0xc0000081	// segment selector
#define MSR_LSTAR		 0xc0000082	// syscall entry point
#define MSR_SYSCALL_MASK 0xc0000084	// flag mask

extern void syscall_entry(void);

extern int sys_exit(void);
extern int sys_exec(void);
extern int sys_write(void);
extern int sys_cwrite(void);


void syscall_init(void){

	uint64_t efer = rdmsr(MSR_EFER);
	wrmsr(MSR_EFER, efer | (1 << 0));
	wrmsr(MSR_LSTAR, (uintptr_t)syscall_entry);
	wrmsr(MSR_STAR, ((uint64_t)0x1b << 48) | ((uint64_t)0x08 << 32));
	wrmsr(MSR_SYSCALL_MASK, ~FL_IF);
}

int (*syscalls[])(void) = {
	[SYS_exit]	sys_exit,
	[SYS_exec]	sys_exec,
	[SYS_write] sys_write,
	[SYS_cwrite] sys_cwrite,
};

void syscall_centry(struct trapframe *tf){

	int num;

	num = tf->rax;
	if (num > 0 && syscalls[num]){
		tf->rax = syscalls[num]();
	} else {
		cprintf("unknown system call \n");
		tf->rax = -1;
	}
}

int fetchint(uintptr_t val, int *ip){
	*ip = val;	

	return 0;
}

int fetchint1(uintptr_t addr, int *ip){

	struct proc *p = myproc();

	if (addr >= p->size || addr + 8 > p->size)
		return -1;

	*ip = *(int*)addr;
	return 0;
}

int argint(int n, int *ip){
	if (n == 0)
		return fetchint(myproc()->tf->rdi, ip);	
	if (n == 1)
		return fetchint(myproc()->tf->rsi, ip);	
	if (n == 2)
		return fetchint(myproc()->tf->rdx, ip);		

	return -1;	
}

int fetchstr(uintptr_t addr, char **pp){

	*pp = (char*)addr;	

	return 0;
}

int argstr(int n, char **pp){

	int addr;
	if (argint(n, &addr) < 0)
		return -1;
		
	return fetchstr(addr, pp);
}
