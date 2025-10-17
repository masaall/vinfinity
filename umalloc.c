
#include "types.h"
#include "user.h"

typedef long Align;

union header {
	struct {
		union header *ptr;
		uintptr_t size;
	} s;
};

typedef union header Header;

Header base;
Header *freep;

void free(void *ap){

	Header *bp, *p;

	bp = (Header*)ap - 1;
	for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
		if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
			break;

	if (bp + bp->s.size == p->s.ptr){
		
	} else {
		bp->s.ptr = p->s.ptr;
	}
	if (p + p->s.size == bp){
		
	} else {
		p->s.ptr = bp;
	}
}

Header *morecore(uintptr_t nunits){

	char *p;
	Header *hp;

	if (nunits < 8)
		nunits = 8;

	if ((p = sbrk(nunits * sizeof(Header))) == (char*)-1)
		return 0;

	hp = (Header*)p;
	hp->s.size = nunits;
	free(hp + 1);

	return freep;
}

void *malloc(uintptr_t nbytes){

	Header *prevp, *p;
	uintptr_t nunits;

	nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
	if ((prevp = freep) == 0){
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}
	for (p = prevp->s.ptr;; p = p->s.ptr){
		if (p->s.size >= nunits){
			if (p->s.size == nunits){
				prevp->s.ptr = p->s.ptr;
			} else {
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			return p + 1;
		}
		if (p == freep)
			if ((p = morecore(nunits)) == 0)
				return 0;		
	}
}
