
struct file {
	enum { FD_NONE, FD_PIPE, FD_INODE } type;
	int ref;
	char readable;
	char writable;
	struct inode *ip;
	uint32_t off;
};

struct inode {
	uint32_t dev;
	uint32_t inum;

	int ref;
	struct sleeplock lock;
	int valid;

	short type;
	short major;
	short minor;
	short nlink;
	uint32_t size;
	uint32_t addr[NDIRECT+1];
};

struct devsw {
	int (*read)(struct inode*, char*, int);
	int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
