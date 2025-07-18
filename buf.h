
struct buf {
	int32_t flags;
	uint32_t dev;
	uint32_t blockno;
	struct sleeplock lock;
	uint32_t refcnt;
	struct buf *prev;
	struct buf *next;
	struct buf *qnext;
	uint8_t data[BSIZE];
};

#define B_VALID	0x2		// buffer has been read from disk
#define B_DIRTY 0x4		// buffer needs to be written to disk
