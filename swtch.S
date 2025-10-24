
.globl swtch
swtch:
	push %rbp
	push %rbx
	push %r12
	push %r13
	push %r14
	push %r15

	mov %rsp,(%rdi)
	mov (%rsi),%rsp

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %rbx
	pop %rbp
	ret

.globl save_context
save_context:
	movq %r15,0(%rdi)
	movq %r14,8(%rdi)
	movq %r13,16(%rdi)
	movq %r12,24(%rdi)
	movq %rbx,32(%rdi)	
	movq %rbp,40(%rdi)

	movq 0(%rsi),%r15
	movq 8(%rsi),%r14
	movq 16(%rsi),%r13
	movq 24(%rsi),%r12
	movq 32(%rsi),%rbx
	movq 40(%rsi),%rbp
	ret
