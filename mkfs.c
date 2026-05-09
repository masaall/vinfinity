
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "param.h"
#include "fs.h"
#include "stat.h"

#define NINODES 200

#define min(a, b) ((a) < (b) ? (a) : (b))

// disk layout:
// [boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE/(BSIZE*8) + 1;
int ninodeblocks = NINODES/IPB + 1;
int nlog = LOGSIZE; 
int nmeta;	// number of meta blocks (boot, sb, log, inode, bitmap)
int nblocks; // number of data blocks

int fsfd;
struct superblock sb;
char zeroes[BSIZE];
uint32_t freeinode = 1;
uint32_t freeblock;

void wsect(uint32_t, void*);
void rsect(uint32_t, void*);
void rinode(uint32_t, struct dinode*);
void winode(uint32_t, struct dinode*);
uint32_t ialloc(uint16_t);
void iappend(uint32_t, void*, uint32_t);

int main(int argc, char *argv[]){

	int i, fd, cc;
	char buf[BSIZE];
	uint32_t rootino, inum;
	struct dirent de;

	if (argc < 2){
		fprintf(stderr, "Usage: mkfs fs.img files...\n");
		exit(1);
	}

	fsfd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fsfd < 0){
		perror(argv[1]);
		exit(1);
	}

	nmeta = 2 + nlog + ninodeblocks + nbitmap;
	nblocks = FSSIZE - nmeta;

	sb.size = FSSIZE;
	sb.nblocks = nblocks;
	sb.ninodes = NINODES;
	sb.nlog = nlog;
	sb.logstart = 2;
	sb.inodestart = 2+nlog;
	sb.bmapstart = 2+nlog+ninodeblocks;

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

		if (argv[i][0] == '_')
			++argv[i];

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

	exit(0);
}

void wsect(uint32_t sector, void *buf){

	if (lseek(fsfd, sector*BSIZE, 0) != sector*BSIZE){
		perror("lseek");
		exit(1);
	}

	if (write(fsfd, buf, BSIZE) != BSIZE){
		perror("write");
		exit(1);
	}
}

void rsect(uint32_t sector, void *buf){
	if (lseek(fsfd, sector*BSIZE, 0) != sector*BSIZE){
		perror("lseek");
		exit(1);
	}

	if (read(fsfd, buf, BSIZE) != BSIZE){
		perror("write");
		exit(1);
	}
}

void rinode(uint32_t inum, struct dinode *dip){

	struct dinode *ip;
	char buf[BSIZE];
	uint32_t blockno;
	
	blockno = IBLOCK(inum, sb);
	rsect(blockno, buf); 
	ip = (struct dinode*)buf + inum % IPB;
	memmove(dip, ip, sizeof(struct dinode));
}

void winode(uint32_t inum, struct dinode *dip){

	struct dinode *ip;
	char buf[BSIZE];
	uint32_t blockno;

	blockno = IBLOCK(inum, sb);
	rsect(blockno, buf);
	ip = (struct dinode*)buf + inum % IPB;
	memmove(ip, dip, sizeof(struct dinode));
	wsect(blockno, buf);
}

uint32_t ialloc(uint16_t type){

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

	uint8_t *p = xp;
	struct dinode din;
	uint32_t off, fbn, blockno;
	char buf[512];

	rinode(inum, &din);
	off = din.size;
	
	fbn = off/BSIZE;
	if (fbn < NDIRECT){
		if (din.addrs[fbn] == 0){
			din.addrs[fbn] = freeblock++;
		}
		blockno = din.addrs[fbn];
	} else {

	}

	rsect(blockno, buf);
	bcopy(p, buf + off%BSIZE, n);
	wsect(blockno, buf);

//	printf("append inum %d at off %d fbn %d sz %d \n", inum, off, fbn, n);

	off += n;

	din.size = off;
	winode(inum, &din);
}

