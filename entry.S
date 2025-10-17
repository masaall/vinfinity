
#include "param.h"
#include "memlayout.h"
#include "mmu.h"

.globl _start
_start = V2P_WO(entry)
entry:

	movq $V2P_WO(pml4t),%rdi
	movq $V2P_WO(pdpt+(PTE_P|PTE_W)),0(%rdi)
	movq $V2P_WO(pdpt1+(PTE_P|PTE_W)),4088(%rdi)

	movq $V2P_WO(pdpt),%rdi
	movq $V2P_WO(pgdir+(PTE_P|PTE_W)),0(%rdi)

	movq $V2P_WO(pdpt1),%rdi
	movq $V2P_WO(pgdir+(PTE_P|PTE_W)),4080(%rdi)

	movq $V2P_WO(pgdir),%rdi
	movq $(0x00000000+(PTE_P|PTE_W|PTE_PS)),0(%rdi)
	movq $(0x00200000+(PTE_P|PTE_W|PTE_PS)),8(%rdi)

	movq $V2P_WO(pml4t),%rax
	movq %rax,%cr3

	movq $(stack + 4096),%rsp

	movq $main,%rax
	jmp *%rax

.align PGSIZE
pml4t:
	.skip PGSIZE
pdpt:
	.skip PGSIZE
pdpt1:
	.skip PGSIZE
pgdir:
	.skip PGSIZE

.comm stack, KSTACKSIZE
