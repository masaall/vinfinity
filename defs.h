
struct context;
struct file;
struct inode;
struct proc;
struct regs;
struct sleeplock;
struct spinlock;

// buf.c
void bufinit(void);
struct buf *bufread(uint32_t, uint32_t);
void brelse(struct buf*);

// console.c
void cls(void);
void consoleinit(void);
void consintr(int (*)(void));
int consolewrite(struct inode*, char*, int);
void cprintf(const char*, ...);
void panic(char*) __attribute__((noreturn));

// exec.c
int exec(char*, char**);

// file.c
struct file *filealloc(void);
void fileclose(struct file*);
struct file *filedup(struct file*);
void fileinit(void);
int filewrite(struct file*, char*, int);

// fs.c
int dirlink(struct inode*, char*, uint32_t);
struct inode *ialloc(uint32_t, short);
struct inode *idup(struct inode*);
void iinit(uint32_t);
void ilock(struct inode*);
void iunlock(struct inode*);
void iunlockput(struct inode*);
void iupdate(struct inode*);
struct inode *namei(char*);
struct inode *nameiparent(char*, char*);
int readi(struct inode*, void*, uint32_t, uint32_t);
int writei(struct inode*, void*, uint32_t, uint32_t);

// gdt.c
void gdtinstall(void);
void tssinstall(struct proc*);

// ide.c
void ideinit(void);
void ideintr(void);
void iderw(struct buf*);

// idt.c
void idtinit(void);
void idtinstall(void);
void picinit(void);

// ioapic.c
extern uint8_t ioapicid;
void ioapicenable(int, int);
void ioapicinit(void);

// kalloc.c
void getcallerpcs(uintptr_t*, uintptr_t*);
void *kalloc(void);
void kfree(void*);
void kinit1(void*, void*);
void kinit2(void*, void*);

// kbc.c
void kbdintr(void);

// lapic.c
extern uint32_t *lapic;
void lapiceoi(void);
int lapicid(void);
void lapicinit(void);
void lapicstartap(uint32_t, uintptr_t);

// mp.c
void mpinit(void);

// sleeplock.c
void acquiresleep(struct sleeplock*);
void initsleeplock(struct sleeplock*, char*);
void releasesleep(struct sleeplock*);
int holdingsleep(struct sleeplock*);

// spinlock.c
void acquire(struct spinlock*);
int holding(struct spinlock*);
void initlock(struct spinlock*, char*);
void release(struct spinlock*);

// swtch.S
void swtch(struct context**, struct context*);

// syscall.c
void syscallinit(void);
void syscall_handler(struct regs*);

// proc.c
void exit(void);
int fork(void);
void forkret(void);
int growproc(int);
struct cpu *mycpu(void);
struct proc *myproc(void);
void pinit(void);
void sched(void);
void scheduler(void) __attribute__((noreturn));
void sleep(void *chan, struct spinlock*);
void userinit(void);
int wait(void);
void wakeup(void*);
void wakeup1(void*);
void yield(void);

// spinlock.c
void pushcli(void);
void popcli(void);

// string.c
int memcmp(const void*, const void*, size_t);
void *memmove(void*, const void*, size_t);
void *memset(void*, int, size_t);
char *safestrcpy(char*, const char*, size_t);
size_t strlen(const char*);
char *strncpy(char*, const char*, size_t);
int strncmp(const char*, const char*, size_t);

// syscall.c
void syscallinit(void);

// uart.c
void uartinit(void);
void uartintr(void);
void uartputc(int);

// vm.c
uintptr_t allocuvm(uintptr_t*, uintptr_t, uintptr_t);
uintptr_t *copyuvm(uintptr_t*, uintptr_t);
uintptr_t deallocuvm(uintptr_t*, uintptr_t, uintptr_t);
void freevm(uintptr_t*);
void inituvm(uintptr_t*, void*, uintptr_t);
void kvminit(void);
int loaduvm(uintptr_t*, struct inode*, uintptr_t, uintptr_t, uintptr_t);
uintptr_t pgaddr(uint16_t, uint16_t, uint16_t,
			uint16_t, uint16_t, uint16_t);
uintptr_t *setupkvm(void);
void switchkvm(void);
void switchuvm(struct proc*);

#define NELEM(x) (sizeof(x)/sizeof(x[0]))

// font.c
void font_str(const char*, size_t, size_t, uint8_t);
// screen.c
void screen_init(void);
void screen_swap(void);

// tetris.c
void render_menu(void);
void tetris(void);
