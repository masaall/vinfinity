
struct buf;
struct context;
struct inode;
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
void 		cprintf(char*, ...);
void 		panic(char*) __attribute__((noreturn));

// fs.c

void ilock(struct inode*);
void iunlock(struct inode*);

// ide.c
void		iderw(struct buf*);

// ioapic.c
extern uint8_t ioapicid;
void 		ioapicinit(void);

// kalloc.c
char 		*kalloc(void);
void 		kfree(char*);
void 		kinit1(void*, void*);

// lapic.c
extern volatile uint32_t *lapic;
int32_t 	lapicid(void);

// mp.c
void 		mpinit(void);

// proc.c
struct cpu 	*mycpu(void);
struct proc *myproc(void);
void		sched(void);
void		sleep(void*, struct spinlock*);
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

// vm.c
void 		kvmalloc(void);
void 		seginit(void);
void 		switchkvm(void);

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
