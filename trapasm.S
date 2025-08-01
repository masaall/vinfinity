
.extern kernelstack

.macro _swapgs
	cmpq	$8, 24(%rsp)
	je 		1f
	swapgs
1:
.endm

.globl alltraps
alltraps:
#	_swapgs
	push	%rax
	push	%rbx
	push	%rcx
	push 	%rdx
	push	%rsi
	push	%rdi
	push	%rbp
	push	%r8
	push 	%r9
	push	%r10
	push	%r11
	push	%r12
	push 	%r13
	push	%r14
	push	%r15

	mov 	%rsp,%rdi
	call	trap

.globl trapret
trapret:
	pop		%r15
	pop		%r14
	pop		%r13
	pop		%r12
	pop		%r11
	pop		%r10
	pop		%r9
	pop		%r8
	pop		%rbp
	pop		%rdi
	pop		%rsi
	pop		%rdx
	pop		%rcx
	pop		%rbx
	pop		%rax

#	_swapgs

	add		$16,%rsp
	iretq
	
	
.globl syscall_entry
syscall_entry:
	swapgs

	mov 	%rsp, userstack
	mov 	kernelstack,%rsp

	pushq	$0x23
	pushq 	userstack
	push	%r11
	pushq	$0x1b
	push 	%rcx

	pushq	$0
	pushq	$0

	push	%rax
	push	%rbx
	pushq	$0
	push	%rdx
	push	%rsi
	push	%rdi
	push	%rbp
	push	%r8
	push	%r9
	push	%r10
	pushq	$0
	push	%r12
	push	%r13
	push	%r14
	push	%r15

	mov 	%rsp,%rdi
	call	syscall_centry

	pop 	%r15
	pop		%r14
	pop		%r13
	pop		%r12
	add 	$8,%rsp
	pop		%r10
	pop		%r9
	pop		%r8
	pop 	%rbp
	pop		%rdi
	pop		%rsi
	pop		%rdx
	add		$8,%rsp
	pop		%rbx
	pop		%rax

	add 	$16,%rsp

	pop 	%rcx
	add 	$8,%rsp
	pop 	%r11
	pop 	%rsp

	swapgs
	sysretq

.data
.globl userstack
userstack:
	.quad 0

