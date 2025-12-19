
#include "syscall.h"
#include "irqs.h"

#define SYSCALL(name) \
	.globl name; \
	name: \
		mov $SYS_##name,%rax;\
		syscall; \
		ret

/*
#define SYSCALL(name) \
	.globl name; \
	name: \
		mov $SYS_##name,%rax;\
		int $T_SYSCALL; \
		ret
*/

SYSCALL(fork)		
SYSCALL(write)
SYSCALL(wait)
SYSCALL(getpid)
SYSCALL(exit)
SYSCALL(sbrk)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(dup)
SYSCALL(exec)
