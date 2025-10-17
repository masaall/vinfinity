
struct cpu {
	uint8_t apicid;
	int ncli;
	int intena;
	struct gdtAll gdt;
	struct proc *proc;
	struct context *context;
	uintptr_t kernelstack;
	uintptr_t userstack;
};

extern struct cpu cpus[];
extern int ncpu;

struct context {
	uintptr_t r15;
	uintptr_t r14;
	uintptr_t r13;
	uintptr_t r12;
	uintptr_t rbx;
	uintptr_t rbp;
	uintptr_t rip;	
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc {
	int pid;
	enum procstate state;
	uintptr_t *pml4t;
	char *kstack;
	char *chan;
	uintptr_t size;
	bool killed;
	struct regs *regs;
	struct context *context;
	struct proc *parent;
	struct inode *cwd;
	struct file *ofile[NOFILE];
};
