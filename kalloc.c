
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"
#include "spinlock.h"

extern char end[];
void freerange(void*, void*);

struct run {
	struct run *next;	
};

struct {
	struct spinlock lock;
	int32_t use_lock;
	struct run *freelist;
} kmem;

void kinit1(void *vstart, void *vend){
	initlock(&kmem.lock, "kmem");
	kmem.use_lock = 0;
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

	if (kmem.use_lock)	
		acquire(&kmem.lock);
	r = (struct run*)v;
	r->next = kmem.freelist;
	kmem.freelist = r;
	if (kmem.use_lock)	
		release(&kmem.lock);
}

char *kalloc(void){

	struct run *r;

	if (kmem.use_lock)
		acquire(&kmem.lock);
	r = kmem.freelist;
	if (r)
		kmem.freelist = r->next;
	if (kmem.use_lock)
		release(&kmem.lock);	

	return (char*)r;	
}

