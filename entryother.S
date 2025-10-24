
.code16
.globl start
start:

	cli

	lgdt gdtdesc
	mov %cr0,%eax
	or $0x1,%eax
	mov %eax,%cr0

	ljmp $0x8,$start32

.code32
start32:	
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss

	movl (start-24),%eax
	movl %eax,%cr3

	mov %cr4,%eax
	or $(1<<5),%eax
	mov %eax,%cr4

	mov $0xc0000080,%ecx
	or $(1<<8),%eax
	wrmsr

	mov %cr0,%eax
	or $(1<<31),%eax
	mov %eax,%cr0

	lgdt gdtdesc1
	ljmp $0x8,$start64

.code64
start64:
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss

	mov (start-24),%rax
	mov %rax,%cr3

	mov (start-8),%rsp

	call *(start-16)

spin:
	jmp spin

gdt:
	.quad 0
	.quad 0x00cf9a000000ffff
	.quad 0x00cf92000000ffff
gdtdesc:
	.word (gdtdesc - gdt - 1)
	.long gdt

gdt1:
	.quad 0
	.quad 0x00a09a0000000000
	.quad 0x00a0920000000000
gdtdesc1:
	.word (gdtdesc1 - gdt1 - 1)
	.quad gdt1	

