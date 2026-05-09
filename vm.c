
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "x86.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

extern char data[];

uintptr_t *kpml5t;

struct kmap {
	uintptr_t vaddr, pstart, pend;
	uint8_t flags;
} kmap[] = {
	{ KERNBASE, 0, EXTMEM, PTE_P | PTE_W},
	{ KERNBASE+EXTMEM, EXTMEM, V2P(data), PTE_P},
	{ (uintptr_t)data, V2P(data), PHYSTOP, PTE_P | PTE_W },
	{ KERNDEV, DEVSPACE, DEVSPACE+0x1000000, PTE_P | PTE_W }
};

void *walkpages(uintptr_t *pml5t, uintptr_t vaddr, bool alloc){

	uintptr_t *pml4t, *pdpt, *pgdir, *pgtab;

	if (pml5t[PML5X(vaddr)] & PTE_P){
		pml4t = P2V(PG_ADDR(pml5t[PML5X(vaddr)]));
	} else {
		if (!alloc || (pml4t = kalloc()) == NULL)
			return NULL;

		memset(pml4t, 0, PGSIZE);
		pml5t[PML5X(vaddr)] = V2P(pml4t) | PTE_P | PTE_W | PTE_U;
	}

	if (pml4t[PML4X(vaddr)] & PTE_P){
		pdpt = P2V(PG_ADDR(pml4t[PML4X(vaddr)]));
	} else {
		if (!alloc || (pdpt = kalloc()) == NULL)
			return NULL;

		memset(pdpt, 0, PGSIZE);
		pml4t[PML4X(vaddr)] = V2P(pdpt) | PTE_P | PTE_W | PTE_U;
	}

	if (pdpt[PDPTX(vaddr)] & PTE_P){
		pgdir = P2V(PG_ADDR(pdpt[PDPTX(vaddr)]));
	} else {
		if (!alloc || (pgdir = kalloc()) == NULL)
			return NULL;

		memset(pgdir, 0, PGSIZE);
		pdpt[PDPTX(vaddr)] = V2P(pgdir) | PTE_P | PTE_W | PTE_U;
	}

	if (pgdir[PDX(vaddr)] & PTE_P){
		pgtab = P2V(PG_ADDR(pgdir[PDX(vaddr)]));
	} else {
		if (!alloc || (pgtab = kalloc()) == NULL)
			return NULL;

		memset(pgtab, 0, PGSIZE);
		pgdir[PDX(vaddr)] = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
	}

	return &pgtab[PTX(vaddr)];
}

int mappages(uintptr_t *pml5t, uintptr_t vaddr, uintptr_t paddr,
	size_t size, uint8_t flags){

	uintptr_t *pte, vend;

	vend = vaddr + size;
	for (; vaddr < vend; vaddr += PGSIZE, paddr += PGSIZE){
		pte = walkpages(pml5t, vaddr, true);
		if (!pte)
			return -1;

		if (*pte & PTE_P){
			panic("mappages: remap");			
		}

		*pte = paddr | flags;
	}

	return 0;
}

void *setupkvm(void){

	uintptr_t *pml5t;
	struct kmap *k;

	pml5t = kalloc();
	if (!pml5t)
		return NULL;

	memset(pml5t, 0, PGSIZE);
	for (k = kmap; k < &kmap[NELEM(kmap)]; k++){
		if (mappages(pml5t, k->vaddr, k->pstart, k->pend-k->pstart, k->flags) < 0){
			freevm(pml5t, 0);
			return NULL;
		}
	}

	return pml5t;
}

void switchkvm(void){
	lcr3(V2P(kpml5t));
}

void switchuvm(struct proc *p){

	cli();
	lcr3(V2P(p->pml5t));
	tss_install(p);
	sti();
}

void kvminit(void){
	kpml5t = setupkvm();
	if (!kpml5t)
		panic("kvminit");
	switchkvm();
}

void inituvm(uintptr_t *pml5t, void *addr, size_t size){

	uintptr_t *paddr;

	paddr = kalloc();
	if (!paddr)
		panic("inituvm");
	memset(paddr, 0, PGSIZE);
	memmove(paddr, addr, size);
	mappages(pml5t, 0, V2P(paddr), PGSIZE, PTE_P | PTE_W | PTE_U);
}

uintptr_t *copyuvm(uintptr_t *oldpml5t, size_t size){

	uintptr_t vaddr, oldpaddr;
	uintptr_t *newpaddr, *pte;
	uint8_t flags;
	uintptr_t *pml5t;

	if ((pml5t = setupkvm()) == NULL){
		return NULL;
	}
	
	for (vaddr = 0; vaddr < size; vaddr += PGSIZE){
		pte = walkpages(oldpml5t, vaddr, false);
		if (!pte)
			return NULL;
		oldpaddr = PG_ADDR(*pte);
		flags = PG_FLAG(*pte);
		newpaddr = kalloc();
		if (!newpaddr)
			return NULL;
		memset(newpaddr, 0, PGSIZE);
		memmove(newpaddr, P2V(oldpaddr), PGSIZE);
		if (mappages(pml5t, vaddr, V2P(newpaddr), PGSIZE, flags) < 0){
			kalloc_free(newpaddr);
			return NULL;		
		}
	}

	return pml5t;
}

uintptr_t pgaddr(uint16_t pml5x, uint16_t pml4x, uint16_t pdptx,
				 uint16_t pdx, uint16_t ptx, uint16_t offset){

	uintptr_t vaddr;

		vaddr = (uintptr_t)pml5x << PML5SHIFT |
				(uintptr_t)pml4x << PML4SHIFT |
				(uintptr_t)pdptx << PDPTSHIFT |
				(uintptr_t)pdx << PDSHIFT |
				(uintptr_t)ptx << PTSHIFT |
				offset;

	if (((uint64_t)1<<56) & vaddr){
		vaddr |= 0xff00000000000000; 
	} 
	
	return vaddr;
}

uintptr_t allocuvm(uintptr_t *pml5t, uintptr_t oldsz, uintptr_t newsz){

	uintptr_t vaddr, *paddr;

	if (oldsz >= newsz)
		return oldsz;

	for (vaddr = oldsz; vaddr < newsz; vaddr += PGSIZE){
		paddr = kalloc();
		memset(paddr, 0, PGSIZE);
		mappages(pml5t, vaddr, V2P(paddr), PGSIZE, PTE_P|PTE_W|PTE_U);
	}

	return newsz;
}

uintptr_t deallocuvm(uintptr_t *pml5t, uintptr_t oldsz, uintptr_t newsz){

	if (oldsz < newsz)
		return oldsz;

	uintptr_t vaddr, *pte, paddr;

	for (vaddr = newsz; vaddr < oldsz; vaddr += PGSIZE){
		pte = walkpages(pml5t, vaddr, false);
		if (!pte){
			vaddr = pgaddr(PML5X(vaddr), PML4X(vaddr), PDPTX(vaddr),
						  PDX(vaddr)+1, 0, 0) - PGSIZE;
		} else if (*pte & PTE_P){
			paddr = PG_ADDR(*pte);
			kalloc_free(P2V(paddr));
		}
	}

	return newsz;
}

void freevm(uintptr_t *pml5t, size_t size){

	deallocuvm(pml5t, size, 0);

	uintptr_t *pml4t, *pdpt, *pgdir, *pgtab;

	for (size_t pml5x = 0; pml5x < 512; pml5x++){
	 if (pml5t[pml5x] & PTE_P){
	  pml4t = P2V(PG_ADDR(pml5t[pml5x]));

	  for (size_t pml4x = 0; pml4x < 512; pml4x++){
	   if (pml4t[pml4x] & PTE_P){
	    pdpt = P2V(PG_ADDR(pml4t[pml4x]));

		for (size_t pdptx = 0; pdptx < 512; pdptx++){
		 if (pdpt[pdptx] & PTE_P){
		  pgdir = P2V(PG_ADDR(pdpt[pdptx]));

		  for (size_t pdx = 0;pdx < 512; pdx++){
		   if (pgdir[pdx] & PTE_P){
		   	pgtab = P2V(PG_ADDR(pgdir[pdx]));
		   	kalloc_free(pgtab);
		   	
		   }
		  }
		  kalloc_free(pgdir);
		  
		 }
		}
		kalloc_free(pdpt);
		
	   }
	  }
	  kalloc_free(pml4t);
	  
	 }
	}
	kalloc_free(pml5t);
}


int loaduvm(struct inode *ip, uintptr_t *pml5t,
			 uintptr_t vaddr, uintptr_t off, uintptr_t size){

	uintptr_t *pte, paddr, i, n;

	for (i = vaddr; i < size; i += PGSIZE){
		pte = walkpages(pml5t, vaddr+i, false);
		if (!pte)
			panic("loaduvm");
		paddr = PG_ADDR(*pte);

		n = min(PGSIZE, size-i);

		readi(ip, P2V(paddr), off+i, n);
	}

	return 0;	
}
