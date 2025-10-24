
struct buf {
	int flags;
	uint32_t dev;
	uint32_t blockno;
	struct sleeplock lock;
	uint32_t refcnt;
	struct buf *prev;
	struct buf *next;
	struct buf *qnext;
	char data[BSIZE];
};

#define B_VALID 0x2
#define B_DIRTY 0x4
