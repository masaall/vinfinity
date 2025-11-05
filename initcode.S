
#include "syscall.h"
#include "irqs.h"

.globl start
start:
/*
	mov $SYS_exec,%rax
	mov $init,%rdi
	mov $argv,%rsi
	int $T_SYSCALL
*/

	mov $SYS_exec,%rax
	mov $init,%rdi
	mov $argv,%rsi
	syscall

init:
	.string "/init\0"

argv:
	.quad init
	.quad 0	
