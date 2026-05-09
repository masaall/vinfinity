
#include "types.h"
#include "defs.h"
#include "mmu.h"

extern char end[];

struct block {
	struct block *next;
	size_t size;	
};

typedef struct block Block;
static Block *freep;
static Block base;

static void kfree1(void*);

void kinit1(void *kstart, void *kend){

	if (freep == NULL){
		base.next = freep = &base;
		base.size = 0;
	}

	uint8_t *s = kstart;
	uint8_t *e = kend;
	size_t size = (e-s)/sizeof(Block);

	Block *bp = kstart;
	bp->size = size;

	kfree1(bp);
}

void kfree1(void *addr){

	Block *p, *bp;

	bp = addr;
	for (p = freep; !(bp > p && bp < p->next); p = p->next){
		if (p >= p->next && (bp > p || bp < p->next)){
			break;
		}
	}

	if (bp + bp->size == p->next){
		bp->size += p->next->size;
		bp->next = p->next->next;
	} else {
		bp->next = p->next;
	}

	if (p + p->size == bp){
		p->size += bp->size;
		p->next = bp->next;
	} else {
		p->next = bp;
	}

	freep = p;
}

void kalloc_free(void *addr){

	if ((char*)addr < end)
		panic("kfree");

	Block *p, *bp;

	bp = addr;
	bp->size = PGSIZE/sizeof(Block);

	for (p = freep; !(bp > p && bp < p->next); p = p->next){
		if (p >= p->next && (bp > p || bp < p->next)){
			break;
		}
	}

	if (bp + bp->size == p->next){
		bp->size += p->next->size;
		bp->next = p->next->next;
	} else {
		bp->next = p->next;
	}

	if (p + p->size == bp){
		p->size += bp->size;
		p->next = bp->next;
	} else {
		p->next = bp;
	}

	freep = p;
}

void *kalloc(void){

	Block *p, *prevp;
	size_t size = PGSIZE/sizeof(Block);

	prevp = freep;
	for (p = prevp->next;; prevp = p, p = p->next){
		if (p->size >= size){
			if (p->size == size){
				prevp->next = p->next;
			} else {
				p->size -= size;
				p += p->size;
			}
			freep = prevp;
			return p;
		}
		if (p == freep)
			return NULL;
	}
}
