
// void swtch(struct context*, struct context*);

.globl swtch
swtch:

	pop 0(%rdi)
	mov %rsp,8(%rdi)
	mov %rbp,16(%rdi)
	mov %rbx,24(%rdi)
	mov %r12,32(%rdi)
	mov %r13,40(%rdi)
	mov %r14,48(%rdi)
	mov %r15,56(%rdi)

	mov 56(%rsi),%r15
	mov 48(%rsi),%r14
	mov 40(%rsi),%r13
	mov 32(%rsi),%r12
	mov 24(%rsi),%rbx
	mov 16(%rsi),%rbp
	mov 8(%rsi),%rsp
	push 0(%rsi)
	ret
