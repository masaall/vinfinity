
#include "syscall.h"

.globl fork
fork:
	mov $SYS_fork,%rax
	syscall
	ret

.globl write
write:
	mov $SYS_write,%rax
	syscall
	ret

.globl wait
wait:
	mov $SYS_wait,%rax
	syscall
	ret	

.globl getpid
getpid:
	mov $SYS_getpid,%rax
	syscall
	ret

.globl exit
exit:
	mov $SYS_exit,%rax
	syscall
	ret

.globl sbrk
sbrk:
	mov $SYS_sbrk,%rax
	syscall
	ret

.globl open
open:
	mov $SYS_open,%rax
	syscall
	ret

.globl mknod
mknod:
	mov $SYS_mknod,%rax
	syscall
	ret

.globl dup
dup:
	mov $SYS_dup,%rax
	syscall
	ret
	
