
#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "sleeplock.h"
#include "buf.h"

static struct buf buf[NBUF];
static struct buf base;

void binit(void){

	struct buf *b;

	base.next = &base;
	base.prev = &base;
	for (b = buf; b < &buf[NBUF]; b++){
		b->next = base.next;
		b->prev = &base;
		base.next->prev = b;
		base.next = b;
	}
}

struct buf *bget(uint32_t dev, uint32_t blockno){

	struct buf *b;

	for (b = base.next; b != &base; b = b->next){
		if (b->dev == dev && b->blockno == blockno){
			b->refcnt++;
			acquiresleep(&b->lock);
			return b;
		}
	}

	for (b = base.prev; b != &base; b = b->prev){
		if (b->refcnt == 0){
			b->dev = dev;
			b->blockno = blockno;
			b->flags = 0;
			b->refcnt = 1;
			acquiresleep(&b->lock);
			return b;
		}
	}

	panic("bget: no buffers");
}

struct buf *bread(uint32_t dev, uint32_t blockno){

	struct buf *b = bget(dev, blockno);
	if ((b->flags & B_VALID) == 0){
		iderw(b);
	}

	return b;
}

void brelse(struct buf *b){

	if (!holdingsleep(&b->lock))
		panic("brelse");

	releasesleep(&b->lock);

	b->refcnt--;
	if (b->refcnt == 0){
		b->next->prev = b->prev;
		b->prev->next = b->next;
		b->next = base.next;
		b->prev = &base;
		base.next->prev = b;
		base.next = b;
	}
}
