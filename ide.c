
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "irqs.h"
#include "fs.h"
#include "buf.h"
#include "x86.h" 

#define IDE_CMD_READ 	0x20
#define IDE_CMD_WRITE 	0x30

struct spinlock idelock;

bool havedisk1;
struct buf *idequeue;

void idewait(void){
	while ((inb(0x1f7) & 0xc0) != 0x40);
}

void ideinit(void){

	initlock(&idelock, "ide");
	ioapicenable(IRQ_IDE, ncpu - 1);
	idewait();

	outb(0x1f6, 0xe0 | (1 << 4));
	for (int i = 0; i < 1000; i++){
		if (inb(0x1f7) != 0){
			havedisk1 = true;
			break;
		}
	}
	outb(0x1f6, 0xe0 | (0 << 4));
}

void idestart(struct buf *b){

	idewait();
	outb(0x1f2, 1);
	outb(0x1f3, b->blockno);
	outb(0x1f6, 0xe0 | (1 << 4));

	if (b->flags & B_DIRTY){
		outb(0x1f7, IDE_CMD_WRITE);
		outsl(0x1f0, b->data, BSIZE/4);
	} else {
		outb(0x1f7, IDE_CMD_READ);
	}
}

void ideintr(void){

	struct buf *b;

	acquire(&idelock);

	if ((b = idequeue) == 0){
		release(&idelock);
		return;
	}

	if (!(b->flags & B_DIRTY))
		insl(0x1f0, b->data, BSIZE/4);

	b->flags |= B_VALID;
	b->flags &= ~B_DIRTY;

	wakeup(b);	

	release(&idelock);
}

void iderw(struct buf *b){

	if (!holdingsleep(&b->lock))
		panic("iderw: buf not locked");
	if (b->dev != 0 && !havedisk1)
		panic("iderw: ide disk 1 not present");

	acquire(&idelock);	

	idequeue = b;

	idestart(b);

	if ((b->flags & B_VALID) != B_VALID){
		sleep(b, &idelock);
	}
	release(&idelock);
}


