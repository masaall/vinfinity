
struct spinlock {
	uint32_t locked;	// is the lock held?

	char *name;			// name of lock
	struct cpu *cpu;	// the cpu holding the lock
	uint64_t pcs[10];	// the call stack
};
