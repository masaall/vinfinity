
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"
#include "spinlock.h"

extern char end[];

void freerange(char*, char*);

struct free {
	struct free *next;
};

struct {
	struct spinlock lock;
	int use_lock;
	struct free *freep;	
} kmem;

void kinit1(void *start, void *end){
	initlock(&kmem.lock, "kmem");
	kmem.use_lock = 0;
	freerange(start, end);
}

void kinit2(void *start, void *end){
	freerange(start, end);
	kmem.use_lock = 1;
}

void freerange(char *start, char *end){
	start = (char*)PGUP((uintptr_t)start);
	for (; start < end; start += PGSIZE)
		kfree(start);
}

void kfree(char *addr){

	struct free *free;

	if ((uintptr_t)addr % PGSIZE || addr < end || V2P(addr) >= PHYSTOP)
		panic("kfree");

	if (kmem.use_lock)
		acquire(&kmem.lock);

	free = (struct free*)addr;
	free->next = kmem.freep;
	kmem.freep = free;

	if (kmem.use_lock)
		release(&kmem.lock);
}

void *kalloc(void){

	struct free *free;

	if (kmem.use_lock)
		acquire(&kmem.lock);

	free = kmem.freep;
	if (free){
		kmem.freep = free->next;
		memset(free, 0, PGSIZE);
	} 

	if (kmem.use_lock)
		release(&kmem.lock);

	return free;
}

void getcallerpcs(uintptr_t *rbp, uintptr_t *pcs){

	uint8_t i;

	for (i = 0; i < 10; i++){
		if (rbp == 0 || rbp < (uintptr_t*)KERNBASE)
			break;
		pcs[i] = *(rbp + 1);
		rbp = (uintptr_t*)*rbp;
	}
	for (; i < 10; i++)
		pcs[i] = 0;
}
