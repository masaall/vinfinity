
#include "syscall.h"

.globl start
start:

	mov $SYS_exec,%rax
	mov $init,%rdi
	mov $argv,%rsi
	syscall

init:
	.string "/init\0"

argv:
	.quad init
	.quad 0	
