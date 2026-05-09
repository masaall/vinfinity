
#include "mmu.h"

.code16
.globl stage2
stage2:

	mov $gdtdesc,%eax
	lgdt (%eax)

	mov %cr0,%eax
	or $(1<<0),%eax
	mov %eax,%cr0

	ljmp $0x8,$start32

.code32
start32:

	mov $pml4t,%edi
	movl $(pdpt + PTE_P|PTE_W),(%edi)

	mov $pdpt,%edi
	movl $(PTE_P|PTE_W|PTE_PS),(%edi)

	mov $pml4t,%eax
	mov %eax,%cr3

	mov %cr4,%eax
	or $(1<<5),%eax
	mov %eax,%cr4

	mov $0xc0000080,%ecx
	rdmsr
	or $(1<<8),%eax
	wrmsr

	mov %cr0,%eax
	or $(1<<31),%eax
	mov %eax,%cr0

	mov $gdtdesc1,%eax
	lgdt (%eax)

	ljmp $0x8,$start64

.code64
start64:

	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss

	call bootmain

spin:
	jmp spin

.code64
print:
	mov $0xb8000,%rdi
print_start:
	mov (%rsi),%al
	inc %rsi
	or %al,%al
	jz print_end
	mov $0x0f,%ah
	mov %ax,(%rdi)
	add $2,%rdi
	jmp print_start
print_end:
	ret

gdt:
	.quad 0
	.quad 0x00cf9a000000ffff
	.quad 0x00cf92000000ffff
gdtdesc:
	.word gdtdesc - gdt - 1
	.long gdt

gdt1:
	.quad 0
	.quad 0x00a09a0000000000
	.quad 0x0000920000000000
gdtdesc1:
	.word gdtdesc1 - gdt1 - 1
	.quad gdt1

hello:
	.asciz "hello world"

.align PGSIZE
pml4t:
	.zero PGSIZE
pdpt:
	.zero PGSIZE
