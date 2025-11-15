
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "buf.h"
#include "stat.h"

#define min(a, b)	((a) < (b) ? (a) : (b))

struct superblock sb;

void readsb(uint32_t dev){

	struct buf *b;
	b = bufread(dev, 1);
	memmove(&sb, b->data, sizeof(sb));
	brelse(b);
}

struct {
	struct inode inode[NINODE];
	struct spinlock lock;
} icache;

void iinit(uint32_t dev){

	initlock(&icache.lock, "icach");
	for (int i = 0; i < NINODE; i++)
		initsleeplock(&icache.inode[i].lock, "inode");

	readsb(dev);
	cprintf("sb: size %d nblock %d ninode %d nlog %d logstart %d inodestart %d bmapstart %d\n",
		 sb.size, sb.nblock, sb.ninode, sb.nlog, sb.logstart, sb.inodestart, sb.bmapstart);	 
}

struct inode *iget(uint32_t dev, uint32_t inum){

	struct inode *ip, *empty;

	acquire(&icache.lock);

	empty = 0;
	for (ip = icache.inode; ip < &icache.inode[NINODE]; ip++){
		if (ip->ref > 0 && ip->dev == dev && ip->inum == inum){
			ip->ref++;
			release(&icache.lock);
			return ip;
		}
		if (empty == 0 && ip->ref == 0)
			empty = ip;
	}

	if (empty == 0)
		panic("iget: no inode");

	ip = empty;
	ip->dev = dev;
	ip->inum = inum;
	ip->ref = 1;
	ip->valid = 0;
	release(&icache.lock);

	return ip;
}

struct inode *ialloc(uint32_t dev, short type){

	uint32_t inum;
	struct buf *bp;
	struct dinode *dip;

	for (inum = 1; inum < sb.ninode; inum++){
		bp = bufread(dev, IBLOCK(inum, sb));
		dip = (struct dinode*)bp->data + inum%IPB;
		if (dip->type == 0){
			memset(dip, 0, sizeof(*dip));
			dip->type = type;
			brelse(bp);
			return iget(dev, inum);
		}
		brelse(bp);
	}

	panic("ialloc: no inodes");
}

void iupdate(struct inode *ip){

	struct buf *bp;
	struct dinode *dip;

	bp = bufread(ip->dev, IBLOCK(ip->inum, sb));
	dip = (struct dinode*)bp->data + ip->inum%IPB;
	dip->type = ip->type;
	dip->major = ip->major;
	dip->minor = ip->minor;
	dip->nlink = ip->nlink;
	dip->size = ip->size;
	memmove(dip->addr, ip->addr, sizeof(ip->addr));
	brelse(bp);
}

struct inode *idup(struct inode *ip){
	acquire(&icache.lock);
	ip->ref++;
	release(&icache.lock);	
	return ip;
}

void ilock(struct inode *ip){

	struct buf *buf;
	struct dinode *dip;

	if (ip == 0 || ip->ref < 1)
		panic("ilock");

	acquiresleep(&ip->lock);	

	if (ip->valid == 0){
		buf = bufread(ip->dev, IBLOCK(ip->inum, sb));
		dip = (struct dinode*)buf->data + ip->inum%IPB;
		ip->type = dip->type;
		ip->major = dip->major;
		ip->minor = dip->minor;
		ip->nlink = dip->nlink;
		ip->size = dip->size;
		memmove(ip->addr, dip->addr, sizeof(ip->addr));
		brelse(buf);
		ip->valid = 1;
		if (ip->type == 0)
			panic("ilock: no type");
	}
}

void iunlock(struct inode *ip){
	if (ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
		panic("iunlock");

	releasesleep(&ip->lock);	
}

void iput(struct inode *ip){

	acquiresleep(&ip->lock);
	if (ip->valid && ip->nlink == 0){
		acquire(&icache.lock);
		int r = ip->ref; 
		release(&icache.lock);
		if (r == 1){
		
		}
	}

	releasesleep(&ip->lock);	

	acquire(&icache.lock);
	ip->ref--;	
	release(&icache.lock);	
}

void iunlockput(struct inode *ip){
	iunlock(ip);
	iput(ip);
}

uint32_t bmap(struct inode *ip, uint32_t fbn){

	uint32_t blockno, *a;
	struct buf *bp;

	if (fbn < NDIRECT){
		if ((blockno = ip->addr[fbn]) == 0){
		}
		return blockno;
	}

	fbn -= NDIRECT;

	if (fbn < NINDIRECT){
		if ((blockno = ip->addr[NDIRECT]) == 0){	
		}
		bp = bufread(ip->dev, blockno);
		a = (uint32_t*)bp->data;
		if ((blockno = a[fbn]) == 0){
		}
		brelse(bp);
		return blockno;
	}

	panic("bmap: out of range");
}

int readi(struct inode *ip, void *dst, uint32_t off, uint32_t n){

	uint32_t tot, n1;
	struct buf *bp;

	if (ip->type == T_DEV){
		
	}

	if (off > ip->size)
		return -1;

	for (tot = 0; tot < n; tot += n1, off += n1, dst += n1){
		bp = bufread(ip->dev, bmap(ip, off/BSIZE));
		n1 = min(n - tot, BSIZE - off%BSIZE);
		memmove(dst, bp->data + off%BSIZE, n1);
		brelse(bp);
	}

	return n;
}

int writei(struct inode *ip, void *src, uint32_t off, uint32_t n){

	uint32_t tot, n1;
	struct buf *bp;

	if (ip->type == T_DEV){
		if (ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
			return -1;
		return devsw[ip->major].write(ip, src, n);	
	}

	if (off > ip->size)
		return -1;

	for (tot = 0; tot < n; tot += n1, off += n1, src += n1){
		bp = bufread(ip->dev, bmap(ip, off/BSIZE));
		n1 = min(n - tot, BSIZE - off%BSIZE);
		memmove(bp->data + off%BSIZE, src, n1);
		brelse(bp);
	}

	if (n > 0 && off > ip->size){
		ip->size = off;
		iupdate(ip);
	}

	return n;
}

int namecmp(const char *s1, const char *s2){
	return strncmp(s1, s2, DIRSIZ);
}

struct inode *dirlookup(struct inode *ip, char *name){

	struct dirent de;
	uint32_t off;

	for (off = 0; off < ip->size; off += sizeof(de)){
		if (readi(ip, &de, off, sizeof(de)) != sizeof(de))
			panic("dirlookup");
		if (de.inum == 0) continue;	
		if (namecmp(name, de.name) == 0){
			return iget(ip->dev, de.inum);
		}
	}
	
	return 0;
}

int dirlink(struct inode *dp, char *name, uint32_t inum){

	struct inode *ip;
	struct dirent de;
	uint32_t off;

	if ((ip = dirlookup(dp, name)) != 0){
		iput(ip);
		return -1;
	}

	for (off = 0; off < dp->size; off += sizeof(de)){
		if (readi(dp, &de, off, sizeof(de)) != sizeof(de))
			panic("dirlink");
		if (de.inum == 0)
			break;
	}

	strncpy(de.name, name, DIRSIZ);
	de.inum = inum;
	if (writei(dp, &de, off, sizeof(de)) != sizeof(de))
		panic("dirlink");

	return 0;
}

char *skipelem(char *path, char *name){

	char *s;

	while (path[0] == '/') path++;
	if (path[0] == 0) return 0;

	s = path;
	while (path[0] != '/' && path[0] != 0) path++;

	strncpy(name, s, DIRSIZ);
	while (path[0] == '/') path++;

	return path;
}

struct inode *namex(char *path, bool parent, char *name){

	struct inode *ip, *next;

	if (path[0] == '/')
		ip = iget(ROOTDEV, ROOTINO);
	else
		ip = idup(myproc()->cwd);

	while ((path = skipelem(path, name)) != 0){
		ilock(ip);
		if (ip->type != T_DIR){
			iunlockput(ip);
			return 0;
		}

		if (parent && path[0] == 0){
			iunlock(ip);
			return ip;
		}

		if ((next = dirlookup(ip, name)) == 0){
			iunlockput(ip);
			return 0;
		}
		iunlockput(ip);
		ip = next;
	}
	return ip;
}

struct inode *namei(char *path){

	char name[DIRSIZ];
	return namex(path, false, name);
}

struct inode *nameiparent(char *path, char *name){
	return namex(path, true, name);
}

