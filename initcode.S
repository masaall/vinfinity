
.globl start
start:

	mov $6,%rax
	mov $init,%rdi
	mov $argv,%rsi
	syscall

spin:
	jmp spin

init:
	.asciz "/init"

argv:
	.quad init
	.quad init
	.quad 0
