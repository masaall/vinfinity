
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"

extern char end[];
void freerange(void*, void*);

struct run {
	struct run *next;	
};

struct {
	struct run *freelist;
} kmem;

void kinit1(void *vstart, void *vend){
	freerange(vstart, vend);
}

void freerange(void *vstart, void *vend){

	char *p;
	uint32_t addr = 0;
	p = (char*)PGROUNDUP((uintptr_t)vstart);
	for (; p + PGSIZE < (char*)vend; p += PGSIZE, addr++)
		kfree(p);
	cprintf("free %ld pages \n", addr);	
}

void kfree(char *v){

	struct run *r;

	if ((uintptr_t)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
		panic("kfree");

	memset(v, 1, PGSIZE);

	r = (struct run*)v;
	r->next = kmem.freelist;
	kmem.freelist = r;
}

char *kalloc(void){

	struct run *r;

	r = kmem.freelist;
	if (r)
		kmem.freelist = r->next;

	return (char*)r;	
}

