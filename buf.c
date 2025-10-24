
#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

struct {
	struct buf buf[NBUF];
	struct buf head;
	struct spinlock lock;
} bcache;

void bufinit(void){

	struct buf *b;

	initlock(&bcache.lock, "bcache");
	bcache.head.next = &bcache.head;
	for (b = bcache.buf; b < &bcache.buf[NBUF]; b++){
		b->next = bcache.head.next;
		b->prev = &bcache.head;
		initsleeplock(&b->lock, "buffer");
		bcache.head.next->prev = b;
		bcache.head.next = b;
	}
}

struct buf *bufget(uint32_t dev, uint32_t blockno){

	struct buf *b;

	acquire(&bcache.lock);

	for (b = bcache.head.next; b != &bcache.head; b = b->next){
		if (b->dev == dev && b->blockno == blockno){
			b->refcnt++;
			release(&bcache.lock);
			acquiresleep(&b->lock);
			return b;
		}
	}

	for (b = bcache.head.prev; b != &bcache.head; b = b->prev){
		if (b->refcnt == 0 && (b->flags&B_DIRTY) == 0){
			b->dev = dev;
			b->blockno = blockno;
			b->flags = 0;
			b->refcnt = 1;
			release(&bcache.lock);
			acquiresleep(&b->lock);
			return b;
		}
	}
	panic("bufget: no buffer");
}

struct buf *bufread(uint32_t dev, uint32_t blockno){

	struct buf *b;

	b = bufget(dev, blockno);
	if ((b->flags&B_VALID) == 0){
		iderw(b);
	}

	return b;
}

void brelse(struct buf *b){

	if (!holdingsleep(&b->lock))
		panic("brelse");

	releasesleep(&b->lock);

	acquire(&bcache.lock);
	b->refcnt--;
	if (b->refcnt == 0){
		b->next->prev = b->prev;
		b->prev->next = b->next;
		b->next = bcache.head.next;
		b->prev = &bcache.head;
		bcache.head.next->prev = b;
		bcache.head.next = b;	
	}
	release(&bcache.lock);	
}
