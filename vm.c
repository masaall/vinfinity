
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "x86.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

extern char data[];
uintptr_t *kpml5t;

uintptr_t *walk_user(uintptr_t *pml5t, uintptr_t vaddr){

	uintptr_t *pml4t, *pdpt, *pgdir, *pgtab;
	uint16_t pml5x = PML5X(vaddr);
	uint16_t pml4x = PML4X(vaddr);
	uint16_t pdpx = PDPX(vaddr);
	uint16_t pdx = PDX(vaddr);
	uint16_t ptx = PTX(vaddr);

	if (!(pml5t[pml5x] & PTE_P))
		return 0;
	pml4t = P2V(PG_ADDR(pml5t[pml5x]));
	if (!(pml4t[pml4x] & PTE_P))
		return 0;
	pdpt = P2V(PG_ADDR(pml4t[pml4x]));
	if (!(pdpt[pdpx] & PTE_P))
		return 0;	
	pgdir = P2V(PG_ADDR(pdpt[pdpx]));
	if (!(pgdir[pdx] & PTE_P))
		return 0;	
	pgtab = P2V(PG_ADDR(pgdir[pdx]));

	return &pgtab[ptx];
}

uintptr_t *walk_kernel(uintptr_t *pml5t, uintptr_t vaddr){

	uintptr_t *pml4t, *pdpt, *pgdir;
	uint16_t pml5x = PML5X(vaddr);
	uint16_t pml4x = PML4X(vaddr);
	uint16_t pdpx = PDPX(vaddr);
	uint16_t pdx = PDX(vaddr);

	if (!(pml5t[pml5x] & PTE_P))
		return 0;
	pml4t = P2V(PG_ADDR(pml5t[pml5x]));
	if (!(pml4t[pml4x] & PTE_P))
		return 0;
	pdpt = P2V(PG_ADDR(pml4t[pml4x]));
	if (!(pdpt[pdpx] & PTE_P))
		return 0;
	pgdir = P2V(PG_ADDR(pdpt[pdpx]));

	return &pgdir[pdx];
}

int map_user(uintptr_t *pml5t, uintptr_t vaddr, uintptr_t paddr,
		    uintptr_t size, uint8_t flags){

	uint16_t pml5x, pml4x, pdpx, pdx, ptx;
	uintptr_t vend;
	uintptr_t *pml4t, *pdpt, *pgdir, *pgtab;

	vend = vaddr + size;
	for (; vaddr < vend; vaddr += PGSIZE, paddr += PGSIZE){
		pml5x = PML5X(vaddr);
		pml4x = PML4X(vaddr);
		pdpx = PDPX(vaddr);
		pdx = PDX(vaddr);
		ptx = PTX(vaddr);

		if (!(pml5t[pml5x] & PTE_P)){
			pml4t = kalloc();
			if (!pml4t) return -1;
			memset(pml4t, 0, PGSIZE);
			pml5t[pml5x] = V2P(pml4t) | PTE_P | PTE_W | PTE_U;
		} else
			pml4t = P2V(PG_ADDR(pml5t[pml5x]));

		if (!(pml4t[pml4x] & PTE_P)){
			pdpt = kalloc();
			if (!pdpt) return -1;
			memset(pdpt, 0, PGSIZE);
			pml4t[pml4x] = V2P(pdpt) | PTE_P | PTE_W | PTE_U;
		} else
			pdpt = P2V(PG_ADDR(pml4t[pml4x]));

		if (!(pdpt[pdpx] & PTE_P)){
			pgdir = kalloc();
			if (!pgdir) return -1;
			memset(pgdir, 0, PGSIZE);
			pdpt[pdpx] = V2P(pgdir) | PTE_P | PTE_W | PTE_U;
		} else
			pgdir = P2V(PG_ADDR(pdpt[pdpx]));

		if (!(pgdir[pdx] & PTE_P)){
			pgtab = kalloc();
			if (!pgtab) return -1;
			memset(pgtab, 0, PGSIZE);
			pgdir[pdx] = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
		} else
			pgtab = P2V(PG_ADDR(pgdir[pdx]));

		if (pgtab[ptx] & PTE_P) 
			panic("map_user remap");

		pgtab[ptx] = paddr | flags;
	}

	return 0;
}

int map_kernel(uintptr_t *pml5t, uintptr_t vaddr, uintptr_t paddr,
			uintptr_t size, uint8_t flags){

	uint16_t pml5x, pml4x, pdpx, pdx;
	uintptr_t vend;
	uintptr_t *pml4t, *pdpt, *pgdir;

	vend = vaddr + size;
	for (; vaddr < vend; vaddr+=HPGSIZE, paddr+=HPGSIZE){

		pml5x = PML5X(vaddr);
		pml4x = PML4X(vaddr);
		pdpx = PDPX(vaddr);
		pdx = PDX(vaddr);

		if (!(pml5t[pml5x] & PTE_P)){
			pml4t = kalloc();
			if (!pml4t) return -1;
			memset(pml4t, 0, PGSIZE);
			pml5t[pml5x] = V2P(pml4t) | PTE_P | PTE_W;
		} else 
			pml4t = P2V(PG_ADDR(pml5t[pml5x]));

		if (!(pml4t[pml4x] & PTE_P)){
			pdpt = kalloc();
			if (!pdpt) return -1;
			memset(pdpt, 0, PGSIZE);
			pml4t[pml4x] = V2P(pdpt) | PTE_P | PTE_W;
		} else
			pdpt = P2V(PG_ADDR(pml4t[pml4x]));

		if (!(pdpt[pdpx] & PTE_P)){
			pgdir = kalloc();
			if (!pgdir) return -1;
			memset(pgdir, 0, PGSIZE);
			pdpt[pdpx] = V2P(pgdir) | PTE_P | PTE_W;
		} else
			pgdir = P2V(PG_ADDR(pdpt[pdpx]));

		if (pgdir[pdx] & PTE_P)
			panic("map_kernel remap");

		pgdir[pdx] = paddr | flags;
	}

	return 0;
}

struct kmap {
	uintptr_t vaddr;
	uintptr_t paddr;
	uintptr_t size;
	uint8_t flags;
} kmap[] = {
	{KERNBASE, 0, PHYSTOP, PTE_P | PTE_W | PTE_PS},
	{KERNDEV, DEVSPACE, 0x1000000, PTE_P | PTE_W | PTE_PS}
};

uintptr_t *setupkvm(void){

	uintptr_t *pml5t;
	struct kmap *k;

	if ((pml5t = kalloc()) == 0)
		return 0;

	memset(pml5t, 0, PGSIZE);
	for (k = kmap; k < &kmap[NELEM(kmap)]; k++)
		if (map_kernel(pml5t, k->vaddr, k->paddr, k->size, k->flags) < 0){
			freevm(pml5t);
			return 0;
		}

	return pml5t;
}

void switchkvm(void){
	lcr3(V2P(kpml5t));
}

void switchuvm(struct proc *p){

	if (p == 0 || p->kstack == 0 || p->pml5t == 0)
		panic("switchuvm");

	pushcli();
	tssinstall(p);
	lcr3(V2P(p->pml5t));
	popcli();
}

void kvminit(void){
	kpml5t = setupkvm();
	switchkvm();
}

void inituvm(uintptr_t *pml5t, void *init, uintptr_t size){

	char *paddr = kalloc();
	memset(paddr, 0, PGSIZE);
	memmove(paddr, init, size);	
	map_user(pml5t, 0, V2P(paddr), PGSIZE, PTE_P|PTE_W|PTE_U);
}

uintptr_t *copyuvm(uintptr_t *pml5t, uintptr_t size){

	uintptr_t *npml5t, *pte;
	uintptr_t vaddr, paddr;
	uint8_t flags;
	char *npaddr;
	
	if ((npml5t = setupkvm()) == 0)
		return 0;
	for (vaddr = 0; vaddr < size; vaddr += PGSIZE){
		if ((pte = walk_user(pml5t, vaddr)) == 0)
			panic("copyuvm: pte should exist");
		paddr = PG_ADDR(*pte);
		flags = PG_FLAG(*pte);
		npaddr = kalloc();
		memset(npaddr, 0, PGSIZE);
		memmove(npaddr, P2V(paddr), PGSIZE);
		if (map_user(npml5t, vaddr, V2P(npaddr), PGSIZE, flags) < 0){
			kfree(npaddr);
			goto bad;
		}
	}
	return npml5t;
bad:
	freevm(npml5t);
	return 0;	
}

uintptr_t allocuvm(uintptr_t *pml5t, uintptr_t oldsz, uintptr_t newsz){

	uintptr_t vaddr;
	char *paddr;

	if (newsz < oldsz)
		return oldsz;

	vaddr = PGUP(oldsz);
	for (; vaddr < newsz; vaddr += PGSIZE){
		if ((paddr = kalloc()) == 0){
			cprintf("allocuvm out of memory\n");
			deallocuvm(pml5t, newsz, oldsz);
			return 0;	
		}
		memset(paddr, 0, PGSIZE);
		if ((map_user(pml5t, vaddr, V2P(paddr),
					 PGSIZE, PTE_P|PTE_W|PTE_U)) < 0){
			cprintf("allocuvm out of memory 1\n");
			deallocuvm(pml5t, newsz, oldsz);
			kfree(paddr);		
			return 0;
		}
	}
	return newsz;
}

uintptr_t pgaddr(uint16_t pml5x, uint16_t pml4x, uint16_t pdpx,
			uint16_t pdx, uint16_t ptx, uint16_t offset){
	uintptr_t addr;

	if (pml5x < PML5X(KERNBASE)){
		addr = (uintptr_t)(pml5x) << PML5SHIFT;
		addr |= (uintptr_t)(pml4x) << PML4SHIFT;
		addr |= (uintptr_t)(pdpx) << PDPTSHIFT;
		addr |= (uintptr_t)(pdx) << PDSHIFT;
		addr |= (uintptr_t)(ptx) << PTSHIFT;
		addr |= offset;		
	} else {
		addr = (uintptr_t)(pml5x) << PML5SHIFT;
		addr |= (uintptr_t)(pml4x) << PML4SHIFT;
		addr |= (uintptr_t)(pdpx) << PDPTSHIFT;
		addr |= (uintptr_t)(pdx) << PDSHIFT;
		addr |= (uintptr_t)(ptx) << PTSHIFT;
		addr |= offset;	
		addr |= 0xff00000000000000;
	}

	return addr;
}

uintptr_t deallocuvm(uintptr_t *pml5t, uintptr_t oldsz, uintptr_t newsz){

	uintptr_t *pte;
	uintptr_t vaddr, paddr;

	if (newsz > oldsz)
		return oldsz;

	vaddr = newsz;
	for (; vaddr < oldsz; vaddr += PGSIZE){
		pte = walk_user(pml5t, vaddr);
		if (!pte)
			vaddr = pgaddr(PML5X(vaddr), PML4X(vaddr), PDPX(vaddr), 
					PDX(vaddr) + 1, 0, 0) - PGSIZE;					
		else if (*pte & PTE_P){
			paddr = PG_ADDR(*pte);
			kfree(P2V(paddr));
			*pte = 0;
		}
	}
	return newsz;
}

void freevm(uintptr_t *pml5t){

	uint16_t pml5x, pml4x, pdpx, pdx;
	uintptr_t *pml4t, *pdpt, *pgdir, *pgtab;

	deallocuvm(pml5t, 0x10000, 0);

	for (pml5x = 0; pml5x < NENTRIES; pml5x++){
		if (pml5t[pml5x] & PTE_P){
			pml4t = P2V(PG_ADDR(pml5t[pml5x]));
			pml5t[pml5x] = 0;

			for (pml4x = 0; pml4x < NENTRIES; pml4x++){
				if (pml4t[pml4x] & PTE_P){
					pdpt = P2V(PG_ADDR(pml4t[pml4x]));
					pml4t[pml4x] = 0;

					for (pdpx = 0; pdpx < NENTRIES; pdpx++){
						if (pdpt[pdpx] & PTE_P){
							pgdir = P2V(PG_ADDR(pdpt[pdpx]));
							pdpt[pdpx] = 0;

							for (pdx = 0; pdx < NENTRIES; pdx++){
								if (pgdir[pdx] & PTE_P && pml5x < PML5X(KERNBASE)){
									pgtab = P2V(PG_ADDR(pgdir[pdx]));
									pgdir[pdx] = 0;

									kfree(pgtab);
								}
							}
							kfree(pgdir);
						}	
					}
					kfree(pdpt);
				}
			}
			kfree(pml4t);
		}
	}
	kfree(pml5t);
}

int loaduvm(uintptr_t *pml5t, struct inode *ip,
		 uintptr_t vaddr, uintptr_t offset, uintptr_t size){

	uintptr_t *pte;
	uintptr_t paddr, i;
	int n;

	if ((uintptr_t)vaddr % PGSIZE != 0)
		panic("loaduvm: vaddr must be page aligned");
	for (i = 0; i < size; i += PGSIZE){
		if ((pte = walk_user(pml5t, vaddr+i)) == 0)
			panic("loaduvm: address should exist");

		paddr = PG_ADDR(*pte);
		n = min(size - i, PGSIZE);

		if (readi(ip, P2V(paddr), offset+i, n) != n)
			return -1;
	}	 
	return 0;
}
