
#include "syscall.h"
#include "traps.h"

.globl start
start:

	mov 	$SYS_exec,%rax
	mov 	$init,%rdi
	mov		$argv,%rsi
	syscall

spin:
	jmp 	spin

init:
	.string "init\0"

init1:
	.string "init1\0"	

.p2align 2
argv:
	.quad init
	.quad 0
