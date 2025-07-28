
struct buf;
struct context;
struct inode;
struct proc;
struct sleeplock;
struct spinlock;

// bio.c
void		binit(void);
struct buf *bget(uint32_t, uint32_t);
struct buf *bread(uint32_t, uint32_t);
void 		brelse(struct buf*);

// console.c
void 		cls(void);
void 		consoleinit(void);
void 		consoleintr(int32_t (*)(void));
int     	consolewrite1(char*, int);
void 		cprintf(char*, ...);
void 		panic(char*) __attribute__((noreturn));

// exec.c
int 		exec(char*, char**);

// file.c
void 		fileinit(void);

// fs.c

void ilock(struct inode*);
void iunlock(struct inode*);

// gdt.c
void 		gdtinstall(void);
void 		tssinstall(struct proc*);

// ide.c
void		iderw(struct buf*);

// ioapic.c
void ioapicenable(int32_t, int32_t);
extern uint8_t ioapicid;
void 		ioapicinit(void);

// kalloc.c
char 		*kalloc(void);
void 		kfree(char*);
void 		kinit1(void*, void*);

// kbd.c
void 		kbdintr(void);

// lapic.c
extern volatile uint32_t *lapic;
void 		lapiceoi(void);
int32_t 	lapicid(void);

// mp.c
void 		mpinit(void);

// picirq.c
void 		picinit(void);

// proc.c
void 		exit(void);
struct cpu 	*mycpu(void);
struct proc *myproc(void);
void		pinit(void);
void		sched(void);
void 		scheduler(void) __attribute__((noreturn));
void		sleep(void*, struct spinlock*);
void		userinit(void);
void		wakeup(void*);

// sleeplock.c

void 		acquiresleep(struct sleeplock *lock);
int32_t 	holdingsleep(struct sleeplock *lock);
void 		initsleeplock(struct sleeplock *lock, char *name);
void 		releasesleep(struct sleeplock *lock);

// spinlock.c

void		acquire(struct spinlock*);
void 		getcallerpcs(void *v, uint64_t pcs[]);
int32_t 	holding(struct spinlock*);
void 		initlock(struct spinlock*, char*);
void		pushcli(void);
void		popcli(void);
void		release(struct spinlock*);

// string.c

uint32_t 	memcmp(const void*, const void*, size_t);
void 		*memset(void*, int32_t, size_t);
void 		*memmove(void*, const void*, uint64_t);

// swtch.S
void		swtch(struct context**, struct context*);

// syscall.c
int 		argint(int, int*);
int 		argstr(int, char**);
int 	    fetchint(uintptr_t, int*);
int 		fetchint1(uintptr_t, int*);
int 		fetchstr(uintptr_t, char **);
void 		syscall(void);
void 		syscall_init(void);

// trap.c
void 		idtinit(void);
void		tvinit(void);

// vm.c
void 		inituvm(uintptr_t*, char*, uintptr_t);
void 		kvmalloc(void);
uintptr_t 	*setupkvm(void);
void 		switchkvm(void);
void 		switchuvm(struct proc*);

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
