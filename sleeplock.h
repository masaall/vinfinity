
struct sleeplock {
	bool locked;
	struct spinlock lock;

	char *name;
	int pid;	
};
