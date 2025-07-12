

struct cpu {
	uint8_t apicid;			// local APIC ID	
	struct segdesc gdt[NSEGS];	// global descriptor table
	int32_t ncli;				// depth of pushcli nesting
	int32_t intena;				// were interrupts enabled before pushcli?
	struct proc *proc;			// the process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int32_t ncpu;

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc {
	uint64_t size;				// size of process memory
	uintptr_t *pml4;
	enum procstate state;		// proces state
	int32_t pid;				// process id
	struct file *ofile[NOFILE];	// open files
	struct inode *cwd;			// current directory
	char name[16];
};

