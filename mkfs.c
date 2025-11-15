
#include "types.h"
#include "param.h" 
#include "fs.h"
#include "stat.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define NINODES 200

#define min(a, b)	((a) < (b) ? (a) : (b))

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE/(BSIZE*8) + 1;
int ninodeblock = NINODES/IPB + 1;
int nlog = LOGSIZE;
int nmeta;
int nblock;

int fsfd;
struct superblock sb;
char zeroes[BSIZE];
uint32_t freeinode = 1;
uint32_t freeblock;

void wsect(uint32_t, void*);
void rsect(uint32_t, void*);
uint32_t ialloc(short);
void iappend(uint32_t, void*, uint32_t);
void rinode(uint32_t, struct dinode*);
void winode(uint32_t, struct dinode*);

int main(int argc, char *argv[]){

	char buf[BSIZE];
	uint32_t rootino, inum, off;
	struct dirent de;
	struct dinode din;
	int i, fd, cc;

	fsfd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fsfd < 0){
		perror(argv[1]);
		exit(1);
	}

	nmeta = 2 + nlog + ninodeblock + nbitmap;
	nblock = FSSIZE - nmeta;

	sb.size = FSSIZE;
	sb.nblock = nblock;
	sb.ninode = NINODES;
	sb.nlog = nlog;
	sb.logstart = 2;
	sb.inodestart = 2 + nlog;
	sb.bmapstart = 2 + nlog + ninodeblock;

//	printf("nmeta %d (boot, super, log block %d inode block %d, bitmap block %d) block %d total %d \n",
//		 nmeta, nlog, ninodeblock, nbitmap, nblock, FSSIZE);

	freeblock = nmeta;

	for (i = 0; i < FSSIZE; i++)
		wsect(i, zeroes);	

	memset(buf, 0, sizeof(buf));
	memmove(buf, &sb, sizeof(sb));
	wsect(1, buf);

	rootino = ialloc(T_DIR);

	bzero(&de, sizeof(de));
	de.inum = rootino;
	strcpy(de.name, ".");
	iappend(rootino, &de, sizeof(de));

	bzero(&de, sizeof(de));
	de.inum = rootino;
	strcpy(de.name, "..");
	iappend(rootino, &de, sizeof(de));

	for (i = 2; i < argc; i++){
		if ((fd = open(argv[i], 0)) < 0){
			perror(argv[i]);
			exit(1);
		}

		if (*argv[i] == '_') ++argv[i];

		inum = ialloc(T_FILE);

		bzero(&de, sizeof(de));
		de.inum = inum;
		strncpy(de.name, argv[i], DIRSIZ);
		iappend(rootino, &de, sizeof(de));

		while ((cc = read(fd, buf, sizeof(buf))) > 0){
			iappend(inum, buf, cc);
		}
		close(fd);
	}

	rinode(rootino, &din);
	off = din.size;
	off = ((off/BSIZE) + 1) * BSIZE;
	din.size = off;
	winode(rootino, &din);

	exit(0);
}

void wsect(uint32_t sect, void *buf){

	if (lseek(fsfd, sect*BSIZE, 0) != sect*BSIZE){
		perror("lseek");
		exit(1);
	}

	if (write(fsfd, buf, BSIZE) != BSIZE){
		perror("write");
		exit(1);
	}
}

void rsect(uint32_t sect, void *buf){
	if (lseek(fsfd, sect*BSIZE, 0) != sect*BSIZE){
		perror("lseek");
		exit(1);
	}

	if (read(fsfd, buf, BSIZE) != BSIZE){
		perror("read");
		exit(1);
	}
}

void winode(uint32_t inum, struct dinode *ip){

	struct dinode *dip;
	char buf[BSIZE];
	uint32_t sect;

	sect = IBLOCK(inum, sb);
	rsect(sect, buf);
	dip = (struct dinode*)buf + inum%IPB;
	memmove(dip, ip, sizeof(*dip));
	wsect(sect, buf);
}

void rinode(uint32_t inum, struct dinode *ip){

	struct dinode *dip;
	char buf[BSIZE];
	uint32_t sect;

	sect = IBLOCK(inum, sb);
	rsect(sect, buf);
	dip = (struct dinode*)buf + inum%IPB;
	memmove(ip, dip, sizeof(*dip));
}

uint32_t ialloc(short type){

	uint32_t inum = freeinode++;
	struct dinode din;

	bzero(&din, sizeof(din));
	din.type = type;
	din.nlink = 1;
	din.size = 0;
	winode(inum, &din);

	return inum;
}

void iappend(uint32_t inum, void *xp, uint32_t n){

	struct dinode din;
	char buf[BSIZE];
	uint32_t indirect[NINDIRECT];
	uint32_t off, fbn, n1, x;

	rinode(inum, &din);
	off = din.size;
//	printf("append inum %d at off %d fbn %d n %d \n", inum, off, fbn, n);
	while (n > 0){
		fbn = off/BSIZE;
		if (fbn < NDIRECT){
			if (din.addr[fbn] == 0){
				din.addr[fbn] = freeblock++;
			}
			x = din.addr[fbn];
		} else {
			if (din.addr[NDIRECT] == 0){
				din.addr[NDIRECT] = freeblock++;
			}
			rsect(din.addr[NDIRECT], indirect);
			if (indirect[fbn - NDIRECT] == 0){
				indirect[fbn - NDIRECT] = freeblock++;
				wsect(din.addr[NDIRECT], indirect);
			}
			x = indirect[fbn - NDIRECT];
		}
		n1 = min(n, BSIZE-off%BSIZE);
		rsect(x, buf);
		bcopy(xp, buf + off%BSIZE, n1);
		wsect(x, buf);
		n -= n1;
		off += n1;
		xp += n1;
	}
	din.size = off;
	winode(inum, &din);
}
