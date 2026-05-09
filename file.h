
// in-memory copy of an inode
struct inode {

	uint32_t dev;
	uint32_t inum;
	struct sleeplock lock;

	uint16_t type;
	uint16_t major;
	uint16_t minor;
	uint16_t nlink;
	uint32_t size;
	uint32_t addrs[NDIRECT+1];
};
