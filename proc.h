

struct cpu {
	uint8_t apicid;			// local APIC ID	
	struct context *scheduler;	// swtch() here to enter scheduler
	struct segdesc gdt[NSEGS];	// global descriptor table
	int32_t ncli;				// depth of pushcli nesting
	int32_t intena;				// were interrupts enabled before pushcli?
	struct proc *proc;			// the process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int32_t ncpu;

struct context {
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbx;
	uint64_t ebp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc {
	uint64_t size;				// size of process memory
	uintptr_t *pml4;
	enum procstate state;		// proces state
	int32_t pid;				// process id
	struct context *context;	// swtch() here to run process
	void *chan;					// if non-zero, sleeping on chan
	int32_t killed;				// if non-zero, have been killed
	struct file *ofile[NOFILE];	// open files
	struct inode *cwd;			// current directory
	char name[16];
};

