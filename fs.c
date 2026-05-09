
#include "types.h"
#include "defs.h"
#include "param.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "buf.h"
#include "stat.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

struct superblock sb;

void readsb(int dev){

	struct buf *b;

	b = bread(dev, 1);
	memmove(&sb, b->data, sizeof(sb));
	brelse(b);
}

void fsinit(uint32_t dev){

	readsb(dev);
//	cprintf("size %d nblocks %d ninodes %d nlog %d logstart %d inodestart %d bmapstart %d \n",
//	 sb.size, sb.nblocks, sb.ninodes, sb.nlog,
//	  sb.logstart, sb.inodestart, sb.bmapstart);	
}

struct inode *iget(uint32_t dev, uint32_t inum){

	struct inode *ip;

	ip = malloc(sizeof(*ip));

	ip->dev = dev;
	ip->inum = inum;

	return ip;
}

void ilock(struct inode *ip){

	struct buf *bp;
	struct dinode *dp;

	acquiresleep(&ip->lock);

	bp = bread(ip->dev, IBLOCK(ip->inum, sb));
	dp = (struct dinode*)bp->data + ip->inum%IPB;
	ip->type = dp->type;
	ip->major = dp->major;
	ip->minor = dp->minor;
	ip->nlink = dp->nlink;
	ip->size = dp->size;
	memmove(ip->addrs, dp->addrs, sizeof(dp->addrs));
	brelse(bp);
}

void iunlock(struct inode *ip){

	if (!holdingsleep(&ip->lock))
		panic("iunlock");

	releasesleep(&ip->lock);
}

uint32_t bmap(struct inode *ip, uint32_t fbn){

	uint32_t blockno;

	if (fbn < NDIRECT){
		if ((blockno = ip->addrs[fbn]) == 0){
			
		}
		return blockno;
	}

	panic("bmap");
}

int readi(struct inode *ip, char *dst, uint32_t off, uint32_t n){

	uint32_t tot, n1;
	struct buf *bp;	
	
	for (tot=0;tot<n;tot+=n1,off+=n1,dst+=n1){
		bp = bread(ip->dev, bmap(ip, off/BSIZE));
		n1 = min(n-tot, BSIZE-off%BSIZE);
		memmove(dst, bp->data+off%BSIZE, n1);
		brelse(bp);
	}

	return n;
}

int namecmp(const char *s1, const char *s2){
	return strncmp(s1, s2, DIRSIZ);	
}

struct inode *dirlookup(struct inode *dp, char *name){

	uint32_t off, inum;
	struct dirent de;

	for (off = 0; off < dp->size; off += sizeof(de)){
		readi(dp, (char*)&de, off, sizeof(de));

		if (de.inum == 0)
			continue;
		if (namecmp(name, de.name) == 0){
			inum = de.inum;
			return iget(dp->dev, inum);
		}
	}

	return 0;
}

char *skipelem(char *path, char *name){

	char *s;
	int n;

	while (*path == '/')
		path++;

	if (*path == 0){
		return 0;		
	}

	s = path;
	while (*path != '/' && *path != 0)
		path++;

	n = path - s;
	if (n >= DIRSIZ){
		memmove(name, s, DIRSIZ);
	} else {
		memmove(name, s, n);
	}

	return path;
}

struct inode *namex(char *path, char *name){

	struct inode *ip, *next;

	ip = iget(ROOTDEV, ROOTINO);

	while((path = skipelem(path, name)) != 0){
		ilock(ip);

		if ((next = dirlookup(ip, name)) == 0){
			return 0;
		}
		iunlock(ip);
		ip = next;

	}

	return ip;
	
}

struct inode *namei(char *path){

	char name[DIRSIZ];
	return namex(path, name);
}
