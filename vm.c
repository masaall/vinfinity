
#include "types.h"
#include "defs.h" 
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "memlayout.h"
//#include "gdt.h"
#include "x86.h"

extern char data[];
uintptr_t *kpml4;
/*
void gdtinstall(void){
	gdt.pointer.limit = sizeof(gdt.entries) + sizeof(gdt.tss_extra) - 1;
	gdt.pointer.base = (uintptr_t)gdt.entries;	

	asm volatile("lgdt %0" : : "m" (gdt.pointer));
}
*/

uintptr_t *walkpage(uintptr_t *pml4, const void *va, int alloc){

	uintptr_t *pml4e, *pdpte, *pde;

	// current = pml4
	uintptr_t *current = pml4;
	pml4e = current + PML4X(va);
	if (!(*pml4e & PTE_P)){
		if (!alloc) return 0;
		uintptr_t *pdpt = (uintptr_t*)kalloc();
		if (!pdpt) return 0;
		memset(pdpt, 0, PGSIZE);
		*pml4e = V2P(pdpt) | PTE_P | PTE_W | PTE_U;	
	}

	// current = pdpt
	current = P2V(PTE_ADDR(*pml4e));
	pdpte = current + PDPTX(va);
	if (!(*pdpte & PTE_P)){
		if (!alloc) return 0;
		uintptr_t *pgdir = (uintptr_t*)kalloc();
		if (!pgdir) return 0;
		memset(pgdir, 0, PGSIZE);
		*pdpte = V2P(pgdir) | PTE_P | PTE_W | PTE_U;
	}

	// current = pagedir
	current = P2V(PTE_ADDR(*pdpte));
	pde = current + PDX(va);
	if (!(*pde & PTE_P)){
		if (!alloc) return 0;
		uintptr_t *pgtab = (uintptr_t*)kalloc();
		if (!pgtab) return 0;
		memset(pgtab, 0, PGSIZE);
		*pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
	}

	// current = pagetab
	current = P2V(PTE_ADDR(*pde));
	return current + PTX(va);
}

int mappages(uintptr_t *pml4, void *va, uintptr_t size, uintptr_t pa, int perm){

	char *a, *last;
	uintptr_t *pte;
	uint32_t pages_mapped = 0;

	a = (char*)(uintptr_t)va;
	last = (char*)PGROUNDDOWN(((uintptr_t)va) + size - 1);
	for (; ; a += PGSIZE, pa += PGSIZE){
		if ((pte = walkpage(pml4, a, 1)) == 0)
			return -1;
		if (*pte & PTE_P)
			panic("remap");
		*pte = pa | perm | PTE_P;
		pages_mapped++;
		if (a == last)
			break;		
	}
//	cprintf("mapped %d pages\n", pages_mapped);
	return 0;
}

struct kmap {
	void *virt;
	uintptr_t phys_start;
	uintptr_t phys_end;
	int64_t perm;
} kmap[] = {
	{ (void*)KERNBASE, 			   0, EXTMEM, 			  PTE_W},
	{ (void*)KERNLINK, V2P(KERNLINK), V2P(data), 			  0},
	{ (void*)data, 		   V2P(data), PHYSTOP, 			  PTE_W},
	{ (void*)KERNDEV,       DEVSPACE, DEVSPACE+0x1000000, PTE_W},
};

uintptr_t *setupkvm(void){

	uintptr_t *pml4;
	struct kmap *k;

	if ((pml4 = (uintptr_t*)kalloc()) == 0)
		return 0;

	memset(pml4, 0, PGSIZE);
	for (k = kmap; k < kmap + NELEM(kmap); k++)
		if (mappages(pml4, k->virt, k->phys_end - k->phys_start,
				k->phys_start, k->perm) < 0){
		return 0;				
	}
	return pml4;
}

void kvmalloc(void){
	kpml4 = setupkvm();
	switchkvm();
}

void switchkvm(void){
	lcr3(V2P(kpml4));
}

void switchuvm(struct proc *p){
	if (p == 0)
		panic("switchuvm: no process");
	if (p->kstack == 0)
		panic("switchuvm: no kstack");
	if (p->pml4 == 0)
		panic("switchuvm: no pml4");		

	pushcli();

	tssinstall(p);

/*	uintptr_t addr = (uintptr_t)&gdt.tss;

	gdt.entries[5].limit_low = sizeof(gdt.tss);
	gdt.entries[5].base_low = addr & 0xffff;
	gdt.entries[5].base_middle = (addr >> 16) & 0xff;
	gdt.entries[5].base_high = (addr >> 24) & 0xff;
	gdt.tss_extra.base_highest = (addr >> 32) & 0xffffffff;
	gdt.tss.iomb = sizeof(gdt.tss);

	gdt.tss.rsp[0] = (uintptr_t)p->kstack + KSTACKSIZE;

//	asm volatile("ltr %0" : : "r" ((uint16_t)0x2b));

	asm volatile(
			"mov $0x10, %%ax\n"
			"mov %%ax, %%ds\n"
			"mov %%ax, %%es\n"
			"mov %%ax, %%ss\n"
			"mov $0x2b, %%ax\n"
			"ltr %%ax\n"
			::: "rax", "memory"	
	);
*/
	lcr3(V2P(p->pml4));
	popcli();	
}

void inituvm(uintptr_t *pml4, char *init, uintptr_t size){

	char *mem;

	if (size >= PGSIZE)
		panic("inituvm: more than a page");
	mem = kalloc();
	memset(mem, 0, PGSIZE);
	mappages(pml4, 0, PGSIZE, V2P(mem), PTE_W|PTE_U);
	memmove(mem, init, size); 
}
