
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "fs.h"
#include "file.h"
#include "x86.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

extern char data[];
uintptr_t *kpml4t;

uintptr_t *walk_user_page(uintptr_t *pml4t, uintptr_t vaddr, bool alloc){

	uintptr_t *pdpt, *pgdir, *pgtab;

	uint16_t pmlx = PMLX(vaddr);
	uint16_t pdpx = PDPX(vaddr);
	uint16_t pdx = PDX(vaddr);
	uint16_t ptx = PTX(vaddr);

	if (!(pml4t[pmlx] & PTE_P)){
		if (!alloc) return 0;
		pdpt = kalloc();
		pml4t[pmlx] = V2P(pdpt) | PTE_P | PTE_W | PTE_U;
	}

	pdpt = P2V(PG_ADDR(pml4t[pmlx]));
	if (!(pdpt[pdpx] & PTE_P)){
		pgdir = kalloc();
		pdpt[pdpx] = V2P(pgdir) | PTE_P | PTE_W | PTE_U;
	}

	pgdir = P2V(PG_ADDR(pdpt[pdpx]));
	if (!(pgdir[pdx] & PTE_P)){
		pgtab = kalloc();
		pgdir[pdx] = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
	}

	pgtab = P2V(PG_ADDR(pgdir[pdx]));
	return &pgtab[ptx];
}

uintptr_t *walk_kernel_page(uintptr_t *pml4t, uintptr_t vaddr, bool alloc){

	uintptr_t *pdpt, *pgdir;

	uint16_t pmlx = PMLX(vaddr);
	uint16_t pdpx = PDPX(vaddr);
	uint16_t pdx = PDX(vaddr);

	if (!(pml4t[pmlx] & PTE_P)){
		if (!alloc) return 0;
		pdpt = kalloc();
		pml4t[pmlx] = V2P(pdpt) | PTE_P | PTE_W;
	}

	pdpt = P2V(PG_ADDR(pml4t[pmlx]));
	if (!(pdpt[pdpx] & PTE_P)){
		pgdir = kalloc();
		pdpt[pdpx] = V2P(pgdir) | PTE_P | PTE_W;
	}

	pgdir = P2V(PG_ADDR(pdpt[pdpx]));

	return &pgdir[pdx];
}

void map_user(uintptr_t *pml4t, uintptr_t vaddr, uintptr_t paddr,
		    uintptr_t size, uint8_t flags){

	uintptr_t vend;
	uintptr_t *pte;

	vend = vaddr + size;
	for (; vaddr < vend; vaddr += PGSIZE, paddr += PGSIZE){
		pte = walk_user_page(pml4t, vaddr, true);

		if (*pte & PTE_P) panic("map_user remap");

		*pte = paddr | PTE_P | flags;	
	}
}

void map_kernel(uintptr_t *pml4t, uintptr_t vaddr, uintptr_t paddr,
			uintptr_t size, uint8_t flags){

	uintptr_t vend;
	uintptr_t *pde;

	vend = vaddr + size;
	for (; vaddr < vend; vaddr+=0x200000, paddr+=0x200000){
		pde = walk_kernel_page(pml4t, vaddr, true);

		if (*pde & PTE_P) panic("map_kernel remap");

		*pde = paddr | PTE_P | flags;
	}
}

struct kmap {
	uintptr_t vaddr;
	uintptr_t pstart;
	uintptr_t pend;
	uint8_t flags;	
} kmap[] = {
	{KERNBASE, 0, PHYSTOP, PTE_W|PTE_PS},
	{KERNDEV, DEVSPACE, DEVSPACE+0x1000000, PTE_W|PTE_PS}
};

uintptr_t *setupkvm(void){

	uintptr_t *pml4t;
	struct kmap *k;

	pml4t = kalloc();
	for (k = kmap; k < kmap + NELEM(kmap); k++){
		map_kernel(pml4t, k->vaddr, k->pstart, 
			k->pend-k->pstart, k->flags);
	}

	return pml4t;
}

void switchkvm(void){
	lcr3(V2P(kpml4t));
}

void switchuvm(struct proc *p){

	pushcli();
	tssinstall(p);
	lcr3(V2P(p->pml4t));
	popcli();
}

void kvminit(void){
	kpml4t = setupkvm();
	switchkvm();
}

void inituvm(uintptr_t *pml4t, void *init, uintptr_t size){

	char *paddr;
	paddr = kalloc();
	map_user(pml4t, 0, V2P(paddr), PGSIZE, PTE_W|PTE_U);
	memmove(paddr, init, size);
}

uintptr_t *copyuvm(uintptr_t *pml4t, uintptr_t size){

	uintptr_t *npml4t, *pte;
	uintptr_t vaddr, paddr;
	uint8_t flags;
	char *npaddr;
	
	npml4t = setupkvm();
	for (vaddr = 0; vaddr < size; vaddr += PGSIZE){
		pte = walk_user_page(pml4t, vaddr, false);
		paddr = PG_ADDR(*pte);
		flags = PG_FLAG(*pte);
		npaddr = kalloc();
		map_user(npml4t, vaddr, V2P(npaddr), PGSIZE, flags);
		memmove(npaddr, P2V(paddr), PGSIZE);
	}

	return npml4t;
}

uintptr_t allocuvm(uintptr_t *pml4t, uintptr_t oldsz, uintptr_t newsz){

	uintptr_t vaddr;
	char *paddr;

	if (newsz < oldsz)
		return oldsz;

	vaddr = PGUP(oldsz);
	for (; vaddr < newsz; vaddr += PGSIZE){
		paddr = kalloc();
		map_user(pml4t, vaddr, V2P(paddr), PGSIZE, PTE_W|PTE_U);
	}
	return newsz;
}

uintptr_t deallocuvm(uintptr_t *pml4t, uintptr_t oldsz, uintptr_t newsz){

	uintptr_t *pte;
	uintptr_t vaddr, paddr;

	if (newsz > oldsz)
		return oldsz;

	vaddr = newsz;
	for (; vaddr < oldsz; vaddr += PGSIZE){
		pte = walk_user_page(pml4t, vaddr, false);
		if (*pte & PTE_P){
			paddr = PG_ADDR(*pte);
			kfree(P2V(paddr));
			*pte = 0;
		}
	}
	return newsz;
}

void freevm(uintptr_t *pml4t){

	uint16_t pmlx, pdpx, pdx;
	uintptr_t *pdpt, *pgdir, *pgtab;

	deallocuvm(pml4t, 0x10000, 0);

	for (pmlx = 0; pmlx < NENTRIES; pmlx++){
		if (pml4t[pmlx] & PTE_P){
			pdpt = P2V(PG_ADDR(pml4t[pmlx]));

			for (pdpx = 0; pdpx < NENTRIES; pdpx++){
				if (pdpt[pdpx] & PTE_P){
					pgdir = P2V(PG_ADDR(pdpt[pdpx]));

					for (pdx = 0; pdx < NENTRIES; pdx++){
						if (pgdir[pdx] & PTE_P && pmlx < PMLX(KERNBASE)){
							pgtab = P2V(PG_ADDR(pgdir[pdx]));

							kfree((char*)pgtab);
						}
					}
					kfree((char*)pgdir);
				}	
			}
			kfree((char*)pdpt);
		}
	}
	kfree((char*)pml4t);
}

int loaduvm(uintptr_t *pml4t, struct inode *ip,
		 uintptr_t vaddr, uintptr_t off, uintptr_t size){

	uintptr_t *pte;
	uintptr_t paddr;
	int n;

	for (;vaddr < size; vaddr += PGSIZE){
		if ((pte = walk_user_page(pml4t, vaddr, false)) == 0)
			panic("loaduvm: address should exist");

		paddr = PG_ADDR(*pte);
		n = min(size - vaddr, PGSIZE);

		if (readi(ip, P2V(paddr), off+vaddr, n) != n)
			return -1;
	}	 
	return 0;
}
