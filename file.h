
struct file {
	enum { FD_NONE, FD_PIPE, FD_INODE } type;
	int32_t ref;		// reference count
	char readable;
	char writable;
	struct pipe *pipe;
	struct inode *ip;
	uint32_t off;
};

struct inode {
	uint32_t dev;		// device number
	uint32_t inum;		// inode number
	int32_t ref;		// reference count
	struct sleeplock lock;
	int32_t valid;		// inode has been read from disk?

	short type;			// copy of disk inode
	short major;
	short minor;
	short nlink;
	uint32_t size;		// size of file (bytes)
	uint32_t addrs[NDIRECT+1];
};

struct devsw {
	int64_t (*read)(struct inode*, char*, int64_t);
	int64_t (*write)(struct inode*, char*, int64_t);
};

extern struct devsw devsw[];

#define CONSOLE 1
