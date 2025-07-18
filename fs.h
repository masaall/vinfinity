
#define BSIZE 512	// block size

struct superblock {
	uint32_t size;			// size of file system image (blocks)
	uint32_t nblocks;		// number of data blocks
	uint32_t ninodes;		// number of inodes
	uint32_t nlog;			// number of log blocks
	uint32_t logstart;		// block number of first log block
	uint32_t inodestart;	// block number of first inode block
	uint32_t bmapstart;		// block number of first free map block	
};

#define NDIRECT 12

// on disk inode structure
struct dinode {
	short type;				// file type
	short major;			// major device number (T_DEV only)
	short minor;			// minor device number (T_DEV only)
	short nlink;			// number of links to inode in file system
	uint32_t size;			// size of file (bytes)
	uint32_t addrs[NDIRECT+1];	// data block addresses
};

#define IPB		(BSIZE/sizeof(struct dinode))

#define IBLOCK(i, s)	((i) / IPB + s.inodestart)
