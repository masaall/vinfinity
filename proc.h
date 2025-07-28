

struct cpu {
	uint8_t apicid;			// local APIC ID	
	struct context *scheduler;	// swtch() here to enter scheduler 
	int32_t ncli;				// depth of pushcli nesting
	int32_t intena;				// were interrupts enabled before pushcli?
	struct proc *proc;			// the process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int32_t ncpu;

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
	uint64_t size;				// size of process memory
	uintptr_t *pml4;
	char *kstack;				// bottom of kernel stack for this process
	enum procstate state;		// proces state
	int32_t pid;				// process id
	struct trapframe *tf;		// trap frame for current syscall
	struct context *context;	// swtch() here to run process
	void *chan;					// if non-zero, sleeping on chan
	int32_t killed;				// if non-zero, have been killed
	struct file *ofile[NOFILE];	// open files
	struct inode *cwd;			// current directory
	char name[16];
	struct syscallframe *sf;
};

