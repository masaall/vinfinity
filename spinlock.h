
struct spinlock {
	bool locked;

	char *name;
	struct cpu *cpu;
};
