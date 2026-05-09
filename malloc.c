
#include "types.h"
#include "defs.h"
#include "mmu.h"

extern char end[];

struct header {
	struct header *next;
	size_t size;
};

typedef struct header Header;
static Header base;
static Header *freep;

void minit1(void *mstart, void *mend){

	if (freep == NULL){
		base.next = freep = &base;
		base.size = 0;
	}

	uint8_t *s = mstart;
	uint8_t *e = mend;
	size_t size = (e-s)/sizeof(Header);

	Header *bp = mstart;
	bp->size = size;

	free(bp + 1);
}

void free(void *ap){

	if ((char*)ap < end)
		panic("free");

	Header *p, *bp;

	bp = (Header*)ap - 1;

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

void *malloc(size_t nbytes){

	Header *p, *prevp;
	size_t nunits;

	nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;

	prevp = freep;
	for (p = prevp->next;; prevp = p, p = p->next){
		if (p->size >= nunits){
			if (p->size == nunits){
				prevp->next = p->next;
			} else {
				p->size -= nunits;
				p += p->size;
				p->size = nunits;
			}
			freep = prevp;
			return p + 1;
		}
		if (p == freep)
			return NULL;
	}
}
