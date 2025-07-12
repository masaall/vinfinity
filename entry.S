
#include "memlayout.h"

.text
.globl _start
_start = V2P_WO(entry)

.globl entry
entry:

	movq	$V2P_WO(pml4t),%rdi
	movq	$512,%rcx
	xorq	%rax,%rax
	rep		stosq

	movq	$V2P_WO(pml4t),%rdi
	movq	$V2P_WO(pdpt),0(%rdi)
	orq 	$0x3,0(%rdi)

	movq	$V2P_WO(pdpt),%rdi
	movq	$512,%rcx
	xorq	%rax,%rax
	rep		stosq

	movq 	$V2P_WO(pdpt),%rdi
	movq	$V2P_WO(pd + 3),0(%rdi)

	movq	$V2P_WO(pd),%rdi
	movq	$512,%rcx
	xorq	%rax,%rax
	rep		stosq

	movq	$V2P_WO(pd),%rdi
	movq	$0x00000083,0(%rdi)
	movq	$0x00200083,8(%rdi)

	movq	$V2P_WO(pml4t),%rdi
	movq	$V2P_WO(pdpt),4088(%rdi)
	orq		$0x3,4088(%rdi)

	movq	$V2P_WO(pdpt),%rdi
	movq	$V2P_WO(pd + 3),4080(%rdi)

	movq 	$V2P_WO(pml4t),%rax
	movq	%rax,%cr3

	movq	$(stack + 65536),%rsp
#	movq	$(stack + 4096),%rsp

	mov 	$main,%rax
	jmp		*%rax

.align 4096
.globl pml4t
pml4t:
	.skip 4096
.globl pdpt	
pdpt:
	.skip 4096
.globl pd0
pd:
	.skip 4096		
	
.comm stack, 65536
