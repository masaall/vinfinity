
#include "types.h"
#include "defs.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "idt.h"
#include "fs.h"
#include "sleeplock.h"
#include "buf.h"
#include "x86.h"

static bool havedisk1;
static struct buf *idequeue;

void ide_handler(struct registers*);

void ideinit(void){

	ioapicenable(IRQ_IDE, ncpu-1);
	isr_install(T_IRQ0+IRQ_IDE, ide_handler);

	outb(0x1f6, 0xe0 | (1<<4));
	for (int i = 0; i < 1000; i++){
		if (inb(0x1f7) != 0){
			havedisk1 = true;
			break;
		}
	}

	outb(0x1f6, 0xe0 | (0<<4));
}

void idestart(struct buf *b){

	uint32_t sector = b->blockno;

	outb(0x1f2, 1);
	outb(0x1f3, sector);
	outb(0x1f4, sector >> 8);
	outb(0x1f5, sector >> 16);
	outb(0x1f6, 0xe0 | (1<<4) | sector >> 24);

	if (b->flags & B_DIRTY){
		outb(0x1f7, 0x30);
	} else {
		outb(0x1f7, 0x20);
	}
}

void ide_handler(struct registers*){

	struct buf *b;

	b = idequeue;
	if (!b){
		return;
	}
	idequeue = b->qnext;

	insl(0x1f0, b->data, BSIZE/4);

	b->flags &= ~B_DIRTY;
	b->flags |= B_VALID;

	wakeup(b);

	if (idequeue)
		idestart(idequeue);

	lapiceoi();
}

void iderw(struct buf *b){

	struct buf **bb;

	if (!holdingsleep(&b->lock))
		panic("iderw: buf not locked");
	if ((b->flags & (B_DIRTY|B_VALID)) == B_VALID)
		panic("iderw: nothing to do");
	if (b->dev != 0 && havedisk1 == false)
		panic("iderw: ide disk 1 not present");

	b->qnext = NULL;
	for (bb = &idequeue; *bb; bb = &(*bb)->qnext);
	*bb = b;

	if (idequeue == b)
		idestart(b);

	while ((b->flags & (B_DIRTY|B_VALID)) != B_VALID){
		sleep(b);
	}
}
