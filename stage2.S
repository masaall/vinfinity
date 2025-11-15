
#include "mmu.h"

.code16
stage2:

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

	movl $pml4t,%edi
	movl $(pdpt+(PTE_P|PTE_W)),0(%edi)

	movl $pdpt,%edi
	movl $(pgdir+(PTE_P|PTE_W)),0(%edi)

	movl $pgdir,%edi
	movl $(0x00000000+(PTE_P|PTE_W|PTE_PS)),0(%edi)

	movl $pml4t,%eax
	movl %eax,%cr3

	mov %cr4,%eax
	or $(1<<5|1<<12),%eax
	mov %eax,%cr4

	mov $0xc0000080,%ecx
	rdmsr
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

	call bootmain

.code64
print:
	mov $0xb8000,%rbx
.start_print:
	lodsb
	or %al,%al
	jz .end_print
	mov $0x07,%ah
	mov %ax,(%rbx)
	add $2,%rbx
	jmp .start_print
.end_print:	
	ret

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

.align PGSIZE
pml4t:
	.skip PGSIZE
pdpt:
	.skip PGSIZE
pgdir:
	.skip PGSIZE
