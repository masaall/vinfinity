
struct context {
	uintptr_t rip;	// 0
	uintptr_t rsp;	// 8
	uintptr_t rbp;	// 16
	uintptr_t rbx;	// 24
	uintptr_t r12;	// 32
	uintptr_t r13;	// 40
	uintptr_t r14;	// 48
	uintptr_t r15;	// 56
};

struct cpu {
	uint8_t apicid;
	uintptr_t kernelstack;
	uintptr_t userstack;
	struct gdtAll gdt;
	struct context context;
	struct proc *running_proc;
};

extern struct cpu cpus[];
extern int ncpu;

enum procstate { UNUSED, READY, RUNNING, SLEEPING, ZOMBIE };

struct proc {
	int pid;
	size_t size;
	enum procstate state;
	uintptr_t *pml5t;
	uint8_t *kstack;
	void *chan;
	bool killed;
	struct context context;
	struct registers *regs;
	struct proc *parent;
	struct list_elem all_elem;
	struct list_elem ready_elem;
	struct list_elem sleeping_elem;
	struct list_elem child_elem;
	struct list child_list;
};
