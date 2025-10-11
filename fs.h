
#define ROOTINO 1
#define BSIZE 512

struct superblock {
	uint32_t size;
	uint32_t nblock;
	uint32_t ninode;
	uint32_t nlog;
	uint32_t logstart;
	uint32_t inodestart;
	uint32_t bmapstart;	
};

#define NDIRECT 12
#define NINDIRECT (BSIZE/sizeof(uint32_t))
#define MAXFILE (NDIRECT+NINDIRECT)

struct dinode {
	short type;
	short major;
	short minor;
	short nlink;
	uint32_t size;
	uint32_t addr[NDIRECT+1];	
};

#define IPB (BSIZE/sizeof(struct dinode))
#define IBLOCK(i, sb)	(i/IPB + sb.inodestart)

#define DIRSIZ 14

struct dirent {
	uint32_t inum;
	char name[DIRSIZ];
} __attribute__((packed));
