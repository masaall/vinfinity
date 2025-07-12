
struct sleeplock {
	uint32_t locked;		// is the lock held?
	struct spinlock lock;	// spinlock protecting this sleeplock

	char *name;				// name of lock
	int32_t pid;			// process holding lock
};

