
#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "buf.h"

struct superblock sb;

void ilock(struct inode *in){

	struct buf *buf;
	struct dinode *din;

	if (in == 0 || in->ref < 1)
		panic("ilock");

	acquiresleep(&in->lock);	

	if (in->valid == 0){
		buf = bread(in->dev, IBLOCK(in->inum, sb));
		din = (struct dinode*)buf->data + in->inum%IPB;
		in->type = din->type;
		in->major = din->major;
		in->minor = din->minor;
		in->nlink = din->nlink;
		in->size = din->size;
		memmove(in->addrs, din->addrs, sizeof(in->addrs));
		brelse(buf);
		in->valid = 1;
		if (in->type == 0)
			panic("ilock: no type");
	}
}

void iunlock(struct inode *in){
	if (in == 0 || !holdingsleep(&in->lock) || in->ref < 1)
		panic("iunlock");

	releasesleep(&in->lock);	
}
