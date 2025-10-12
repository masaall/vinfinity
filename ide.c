
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "proc.h"
#include "irqs.h"
#include "fs.h"
#include "buf.h"
#include "x86.h" 

#define IDE_CMD_READ 	0x20
#define IDE_CMD_WRITE 	0x30


bool havedisk1;
struct buf *idequeue;

void ideinit(void){

	int i;

	ioapicenable(IRQ_IDE, 0);

	outb(0x1f6, 0xe0 | (1 << 4));
	for (i = 0; i < 10; i++){
		if (inb(0x1f7) != 0){
			havedisk1 = true;
			break;
		}
	}
	outb(0x1f6, 0xe0 | (0 << 4));
}

void idestart(struct buf *b){

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

	if ((b = idequeue) == 0){
		return;
	}

	if (!(b->flags & B_DIRTY))
		insl(0x1f0, b->data, BSIZE/4);

	b->flags |= B_VALID;
	b->flags &= ~B_DIRTY;

	wakeup(b);	
}

void iderw(struct buf *b){

	if (b->dev != 0 && havedisk1 == false)
		panic("iderw: ide disk 1 not present");

	idequeue = b;

	idestart(b);

	if ((b->flags & B_VALID) != B_VALID){
		sleep(b);
	}

}


