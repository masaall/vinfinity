
#define ROOTINO 1
#define BSIZE 512 // Block size

struct superblock {
	uint32_t size;		// Size of file system image (blocks)
	uint32_t nblocks;	// Number of data blocks
	uint32_t ninodes;	// Number of inodes
	uint32_t nlog;		// Number of log blocks
	uint32_t logstart;	// Block number of first log block
	uint32_t inodestart;// Block number of first inode block
	uint32_t bmapstart;	// Block number of first free map block
};

#define NDIRECT 12
#define NINDIRECT (BSIZE/sizeof(uint32_t))
#define MAXFILE (NDIRECT+NINDIRECT)

// On-disk inode structure
struct dinode {
	uint16_t type;		// File type
	uint16_t major;		// Major device number (T_DEV only)
	uint16_t minor;		// Minor device number (T_DEV only)
	uint16_t nlink;		// Number of links to inode in file system
	uint32_t size;		// Size of file (bytes)
	uint32_t addrs[NDIRECT+1];	// Data block addresses
};

// Inodes per block
#define IPB (BSIZE/sizeof(struct dinode))

#define IBLOCK(i, sb) (i/IPB + sb.inodestart)

#define DIRSIZ 14

struct dirent {
	uint16_t inum;
	char name[DIRSIZ];	
};
