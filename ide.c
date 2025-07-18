
#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "buf.h"
#include "x86.h"

#define SECTOR_SIZE		512
#define IDE_BSY			0x80
#define IDE_DRDY		0x40
#define IDE_DF			0x20
#define IDE_ERR			0x01

#define IDE_CMD_READ	0x20
#define IDE_CMD_WRITE	0x30
#define IDE_CMD_RDMUL	0xc4
#define IDE_CMD_WRMUL	0xc5

static struct spinlock idelock;
static struct buf *idequeue;

static int32_t havedisk1;

int32_t idewait(int32_t checkerr){

	int32_t r;

	while (((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY);

	if (checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
		return -1;

	return 0;
}

void idestart(struct buf *b){

	if (b == 0)
		panic("idestart");
	if (b->blockno >= FSSIZE)
		panic("incorrect blockno");
	int32_t sector_per_block = BSIZE/SECTOR_SIZE;
	int32_t sector = b->blockno * sector_per_block;
	int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ : IDE_CMD_RDMUL;
	int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

	if (sector_per_block > 7) panic("idestart");

	idewait(0);
	outb(0x3f6, 0);		// generate interrupt
	outb(0x1f2, sector_per_block);
	outb(0x1f3, sector & 0xff);		
	outb(0x1f4, (sector >> 8) & 0xff);
	outb(0x1f5, (sector >> 16) & 0xff);
	outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector >> 24)&0xf));
	if (b->flags & B_DIRTY){
		outb(0x1f7, write_cmd);
		outsl(0x1f0, b->data, BSIZE/4);
	} else {
		outb(0x1f7, read_cmd);
	}
}

void iderw(struct buf *b){

	struct buf **pp;

	if (!holdingsleep(&b->lock))
		panic("iderw: buf not locked");
	if ((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
		panic("iderw: nothing to do");	
	if (b->dev != 0 && !havedisk1)
		panic("iderw: ide disk 1 not present");	

	acquire(&idelock);

	b->qnext = 0;
	for ( pp = &idequeue; *pp; pp = &(*pp)->qnext);
	*pp = b;
	
	if (idequeue == b)
		idestart(b);

	while ((b->flags & (B_VALID|B_DIRTY)) != B_VALID)
		sleep(b, &idelock);	

	release(&idelock);
}


