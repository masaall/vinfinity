
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "idt.h"
#include "x86.h"

#define offsetof(start,elem) ((uint8_t*)&elem-(uint8_t*)start)

uint64_t nextpid = 1;

static struct list all_list;
static struct list ready_list;
static struct list sleeping_list;

static struct proc *initial_proc;

extern void to_user(void);
void forkret(void);

static uintptr_t offset_all_elem(void){
	return offsetof(initial_proc, initial_proc->all_elem);
}

static uintptr_t offset_ready_elem(void){
	return offsetof(initial_proc, initial_proc->ready_elem);
}

static uintptr_t offset_sleeping_elem(void){
	return offsetof(initial_proc, initial_proc->sleeping_elem);
}

static uintptr_t offset_child_elem(void){
	return offsetof(initial_proc, initial_proc->child_elem);
}

static void set_proc_ready(struct proc *p){
	p->state = READY;
	list_insert(&ready_list, &p->ready_elem);
}

static void set_proc_running(struct cpu *c, struct proc *p){
	p->state = RUNNING;
	c->running_proc = p;
	list_remove(&p->ready_elem);
}

static void set_proc_sleeping(struct proc *p){
	p->state = SLEEPING;
	list_insert(&sleeping_list, &p->sleeping_elem);
}

static void set_proc_wakeup(struct proc *p){
	p->state = READY;
	list_remove(&p->sleeping_elem);
	list_insert(&ready_list, &p->ready_elem);
}

struct cpu *mycpu(void){

	int apicid;

	if (readrflags()&FL_IF)
		panic("mycpu called with interrupt enable");

	apicid = lapicid();
	for (int i = 0; i < ncpu; i++){
		if (cpus[i].apicid == apicid)
			return &cpus[i];
	}

	panic("unknown apicid");
}

struct proc *myproc(void){

	struct proc *p;

	cli();
	p = mycpu()->running_proc;
	sti();

	return p;
}

void proc_init(void){

	list_init(&all_list);
	list_init(&ready_list);
	list_init(&sleeping_list);
}

struct proc *create_proc(void){

	struct proc *p;
	uint8_t *stack;

	p = malloc(sizeof(*p));
	if (!p)
		return NULL;

	p->pid = nextpid++;

	p->kstack = malloc(4096);
	if (!p->kstack){
		free(p);
		return NULL;
	}

	stack = p->kstack + KSTACKSIZE;

	stack -= sizeof(*p->regs);
	p->regs = (void*)stack;

	stack -= 8;
	*(void**)stack = to_user;

	p->context.rip = (uintptr_t)forkret;
	p->context.rsp = (uintptr_t)stack;

	return p;
}

void init_proc(void){

	extern char _binary_initcode_start[], _binary_initcode_size[];
	struct proc *p;

	initial_proc = p = create_proc();
	p->pml5t = setupkvm();
	if (!p->pml5t)
		panic("procinit");
	inituvm(p->pml5t, _binary_initcode_start, (size_t)_binary_initcode_size);
	p->size = PGSIZE;

	p->regs->cs = 0x2b;
	p->regs->ss = 0x23;
	p->regs->rsp = PGSIZE;
	p->regs->rflags = FL_IF;

	list_insert(&all_list, &p->all_elem);
	list_init(&p->child_list);
	set_proc_ready(p);
}

int fork(void){

	struct proc *newproc, *curproc;
	uint64_t pid;

	curproc = myproc();
	newproc = create_proc();
	if (!newproc)
		return -1;

	newproc->pml5t = copyuvm(curproc->pml5t, curproc->size);

	pid = newproc->pid;
	newproc->parent = curproc;
	newproc->size = curproc->size;

	memmove(newproc->regs, curproc->regs, sizeof(*curproc->regs));
	newproc->regs->rax = 0;

	list_insert(&all_list, &newproc->all_elem);
	list_init(&newproc->child_list);

	list_insert(&curproc->child_list, &newproc->child_elem);
	set_proc_ready(newproc);

	return pid;
}

void exit(void){

	struct list_elem *e;
	struct proc *curproc, *p;

	curproc = myproc();

	if (curproc == initial_proc){
		for (e = all_list.base.prev; e != &all_list.base; e = e->prev){
			struct proc *p = (void*)((uintptr_t)e - offset_all_elem());
			p->state = ZOMBIE;
		}
		panic("init exiting");
	}

	wakeup(curproc->parent);

	for (e = curproc->child_list.base.prev;
		 e != &curproc->child_list.base;
		 e = e->prev){

		list_remove(e);
		list_insert(&initial_proc->child_list, e);

		p = (void*)((uintptr_t)e-offset_child_elem());
		p->parent = initial_proc;

		if (p->state == ZOMBIE)
			wakeup(initial_proc);
	}

	curproc->state = ZOMBIE;
	sched();
}

int wait(void){

	struct proc *curproc, *p;
	struct list_elem *e;
	bool havechild;
	int pid;

	curproc = myproc();
	while (true){
		havechild = false;
		for (e = curproc->child_list.base.prev;
			 e != &curproc->child_list.base;
			 e = e->prev){
			havechild = true;
			p = (void*)((uintptr_t)e-offset_child_elem());

			if (p->state == ZOMBIE){
				pid = p->pid;
				free(p->kstack);
				freevm(p->pml5t, p->size);
				list_remove(&p->child_elem);
				list_remove(&p->all_elem);
				memset(p, 0, sizeof(*p));
				free(p);

				return pid;
			}
		}

		if (!havechild){

			return -1;
		}
		sleep(curproc);
	}

	return 0;
}

void sched(void){

	struct cpu *c;
	struct proc *p;

	cli();
	c = mycpu();
	p = myproc();
	swtch(&p->context, &c->context);
	sti();
}

void scheduler(void){

	struct cpu *c = mycpu();
	struct list_elem *e;

	while (true){
		sti();

		for (e = ready_list.base.prev;
			 e != &ready_list.base;
			 e = e->prev){
			struct proc *p = (void*)((uintptr_t)e-offset_ready_elem());
			set_proc_running(c, p);
			switchuvm(p);

			swtch(&c->context, &p->context);

			c->running_proc = NULL;
		}
	}
}

void forkret(void){

	fsinit(ROOTDEV);
}

void yield(void){
	set_proc_ready(myproc());
	sched();
}

void sleep(void *chan){

	struct proc *p = myproc();

	if (!p)
		panic("sleep");

	p->chan = chan;
	set_proc_sleeping(p);
	sched();

	p->chan = NULL;
}

void wakeup(void *chan){

	struct list_elem *e;
	for (e = sleeping_list.base.prev;
		 e != &sleeping_list.base;
		 e = e->prev){
		struct proc *p;
		p = (void*)((uintptr_t)e-offset_sleeping_elem());
		if (p->chan == chan){
			set_proc_wakeup(p);
		}
	}
}

void show_all_proc(void){

	static char *state[] = {
		[UNUSED] "unused",
		[READY] "ready",
		[RUNNING] "running",
		[SLEEPING] "sleeping",
		[ZOMBIE] "zombie",
	};

	struct list_elem *e;
	for (e = all_list.base.prev; e != &all_list.base; e = e->prev){
		struct proc *p;
		p = (void*)((uintptr_t)e-offset_all_elem()); 
		cprintf("pid %d state %s\n", p->pid, state[p->state]);
	}
	cprintf("\n");
}

void show_child_proc(void){

	static char *state[] = {
		[UNUSED] "unused",
		[READY] "ready",
		[RUNNING] "running",
		[SLEEPING] "sleeping",
		[ZOMBIE] "zombie",
	};

	struct list_elem *e;
	struct proc *parentproc = myproc();

	if (!parentproc)
		return;

	cprintf("parent: pid %d state %s \n", parentproc->pid, state[parentproc->state]);
	for (e = parentproc->child_list.base.prev;
		 e != &parentproc->child_list.base;
		 e = e->prev){
		struct proc *childproc;
		childproc = (void*)((uintptr_t)e-offset_child_elem());
		cprintf("child: pid %d state %s\n", childproc->pid, state[childproc->state]);
	}
	cprintf("\n");
}
