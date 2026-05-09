
struct buf;
struct context;
struct inode;
struct list;
struct list_elem;
struct proc;
struct registers;
struct sleeplock;

// bio.c
void binit(void);
struct buf *bread(uint32_t, uint32_t);
void brelse(struct buf*);

// console.c
void cls(void);
void consoleintr(int (*)(void));
int consolewrite1(const char*, size_t);
void cprintf(const char*, ...);
void panic(const char *) __attribute__((noreturn));

// exec.c
int exec(char*, char *[]);

// fs.c
void fsinit(uint32_t dev);
void ilock(struct inode*);
void iunlock(struct inode*);
struct inode *namei(char*);
int readi(struct inode*, char*, uint32_t, uint32_t);

// gdt.c
void gdt_init(void);
void tss_install(struct proc*);

// kbd.c
void keyboard_init();

// ide.c
void ideinit(void);
void iderw(struct buf*);

// idt.c
void idt_init(void);
void isr_init(void);
void isr_install(size_t, void (*)(struct registers*));
void picinit(void);

// ioapic.c
void ioapicenable(int, int);
void ioapicinit(void);

// kalloc.c
void *kalloc(void);
void kalloc_free(void *);
void kinit1(void *, void *);

// lapic.c
extern volatile uint32_t *lapic;
void lapiceoi(void);
int lapicid(void);
void lapic_init(void);

// list.c
void list_init(struct list*);
void list_insert(struct list*, struct list_elem*);
void list_remove(struct list_elem*);

// malloc.c
void free(void *);
void *malloc(size_t);
void minit1(void*, void*);

// mp.c
void mpinit(void);

// proc.c
void exit(void);
int fork(void);
struct cpu *mycpu(void);
struct proc *myproc(void);
void init_proc(void);
void proc_init(void);
void sched(void);
void scheduler(void) __attribute__((noreturn));
void show_all_proc(void);
void show_child_proc(void);
void sleep(void*);
int wait(void);
void wakeup(void*);
void yield(void);

// sleeplock.c
void acquiresleep(struct sleeplock*);
bool holdingsleep(struct sleeplock*);
void releasesleep(struct sleeplock*);

// string.c
int memcmp(const void*, const void*, size_t);
void *memmove(void*, const void*, size_t);
void *memset(void*, int, size_t);
int strncmp(const char*, const char*, size_t);

// swtch.S
void swtch(struct context*, struct context*);

// syscall.c
void syscall_init(void);

// timer.c
void timer_init(void);

// vm.c
uintptr_t allocuvm(uintptr_t*, uintptr_t, uintptr_t);
uintptr_t *copyuvm(uintptr_t*, size_t);
void freevm(uintptr_t*, size_t);
void inituvm(uintptr_t *, void *, size_t );
int loaduvm(struct inode*, uintptr_t*,uintptr_t, uintptr_t, uintptr_t);
uintptr_t pgaddr(uint16_t, uint16_t, uint16_t, 
				 uint16_t, uint16_t, uint16_t);
void kvminit(void);
void *setupkvm(void);
void switchuvm(struct proc*);

#define NELEM(x) (sizeof(x)/sizeof(x[0]))
