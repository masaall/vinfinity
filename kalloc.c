
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"
#include "spinlock.h"

extern char end[];

void freerange(char*, char*);

struct block {
	struct block *next;
	struct block *prev;	
};

struct {
	struct spinlock lock;
	int use_lock;
	struct block head;
} kmem;

void kinit1(void *start, void *end){
	initlock(&kmem.lock, "kmem");
	kmem.use_lock = 0;
	kmem.head.prev = &kmem.head;
	kmem.head.next = &kmem.head;
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

void kfree(void *addr){

	struct block *b;

	if (kmem.use_lock)
		acquire(&kmem.lock);

	if ((uintptr_t)addr % PGSIZE || (char*)addr < end || V2P(addr) >= PHYSTOP)
		panic("kfree");

	b = (struct block*)addr;
	b->next = &kmem.head;
	b->prev = kmem.head.prev;
	kmem.head.prev->next = b;
	kmem.head.prev = b;

	if (kmem.use_lock)
		release(&kmem.lock);
}

void *kalloc(void){

	struct block *b;

	if (kmem.use_lock)
		acquire(&kmem.lock);

	b = kmem.head.next;
	if (b && b != &kmem.head){
		b->next->prev = b->prev;
		b->prev->next = b->next;
		memset(b, 0, PGSIZE);
	} else 
		return 0;

	if (kmem.use_lock)
		release(&kmem.lock);

	return b;
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
