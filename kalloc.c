
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"
#include "spinlock.h"

extern char end[];

void freerange(char*, char*);

struct block {
	struct block *next;
};

static struct {
	struct spinlock lock;
	struct block *head;
	struct block *tail;
	bool use_lock;
} kmem;

void kinit1(void *start, void *end){
	initlock(&kmem.lock, "kmem");
	kmem.use_lock = false;
	freerange(start, end);
}

void kinit2(void *start, void *end){
	freerange(start, end);
	kmem.use_lock = true;
}

void freerange(char *start, char *end){

	start = (char*)PGUP((uintptr_t)start);
	for (; start < end; start += PGSIZE)
		kfree(start);
}

void kfree(void *v){

	struct block *b;

	if (kmem.use_lock)
		acquire(&kmem.lock);

	if ((uintptr_t)v % PGSIZE || (char*)v < end || V2P(v) >= PHYSTOP)
		panic("kfree");

	b = (struct block*)v;
	b->next = NULL;

	if (kmem.tail){
		kmem.tail->next = b;
		kmem.tail = b;
	} else {
		kmem.head = b;
		kmem.tail = b;
	}

	if (kmem.use_lock)
		release(&kmem.lock);
}

void *kalloc(void){

	struct block *b;

	if (kmem.use_lock)
		acquire(&kmem.lock);

	if (kmem.head == NULL){
		if (kmem.use_lock)
			release(&kmem.lock);
		return NULL;
	}

	b = kmem.head;
	kmem.head = b->next;

	if (kmem.head == NULL)
		kmem.tail = NULL;

	memset(b, 0, PGSIZE);
	if (kmem.use_lock)
		release(&kmem.lock);

	return b;
}

void getcallerpcs(uintptr_t *rbp, uintptr_t *pcs){

	int i;

	for (i = 0; i < 10; i++){
		if (rbp == NULL || rbp < (uintptr_t*)KERNBASE)
			break;
		pcs[i] = rbp[1];
		rbp = (uintptr_t*)rbp[0];
	}
	for (; i < 10; i++)
		pcs[i] = 0;
}
