
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "spinlock.h"
#include "irqs.h"
#include "x86.h"
#include "msr.h"

extern void touser(void);
extern void touser1(void);

struct {
	struct proc proc[NPROC];
	struct spinlock lock;
} ptable;

static struct proc *initproc;
int nextpid = 1;

void pinit(void){
	initlock(&ptable.lock, "ptable");
}

struct cpu *mycpu(void){

	int apicid, i;

	if (readrflags()&FL_IF)
		panic("mycpu called with interrupt enabled");

	apicid = lapicid();
	for (i = 0; i < ncpu; i++)
		if (cpus[i].apicid == apicid)
			return &cpus[i];

	panic("unknown apicid");
}

struct proc *myproc(void){

	struct cpu *c;
	struct proc *p;

	pushcli();
	c = mycpu();
	p = c->proc;
	popcli();

	return p;
}

struct proc *allocproc(void){

	struct proc *p;
	char *stack;
	static bool first = true;

	acquire(&ptable.lock);

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == UNUSED)
			goto found;

	release(&ptable.lock);
	return 0;
found:
	p->pid = nextpid++;
	p->state = EMBRYO;

	release(&ptable.lock);

	if ((p->kstack = kalloc()) == 0){
		p->state = UNUSED;
		return 0;
	}

	stack = p->kstack + KSTACKSIZE;

	stack -= sizeof(*p->regs);
	p->regs = (struct regs*)stack;

	stack -= 8;
	if (first){
		first = false;
		*(void**)stack = touser;
	} else {
		*(void**)stack = touser1;
	}

	stack -= sizeof(*p->context);
	p->context = (struct context*)stack;
	p->context->rip = (uintptr_t)forkret;

	return p;
}

void userinit(void){

	struct proc *p;
	extern char _binary_initcode_start[], _binary_initcode_size[];

	p = allocproc();
	initproc = p;

	p->pml4t = setupkvm();
	inituvm(p->pml4t, _binary_initcode_start, (uintptr_t)_binary_initcode_size);
	p->size = PGSIZE;
	p->regs->cs = 0x2b;
	p->regs->ss = 0x23;
	p->regs->rsp = PGSIZE;
	p->regs->rflags = FL_IF;
	p->regs->rip = 0;

	p->cwd = namei("/");

	acquire(&ptable.lock);
	p->state = RUNNABLE;
	release(&ptable.lock);
}

int growproc(int n){

	struct proc *curproc = myproc();
	uintptr_t size;

	size = curproc->size;
	if (n > 0){
		if ((size = allocuvm(curproc->pml4t, size, size + n)) == 0)
			return -1;
	} else if (n < 0) {
		if (( size = deallocuvm(curproc->pml4t, size, size + n)) == 0)
			return -1;
	}
	curproc->size = size;

	return 0;
}

int fork(void){

	struct proc *newproc, *curproc = myproc();
	int pid, i;

	if ((newproc = allocproc()) == 0)
		return -1;

	if ((newproc->pml4t = copyuvm(curproc->pml4t, curproc->size)) == 0){
		kfree(newproc->kstack);
		newproc->kstack = 0;
		newproc->state = UNUSED;
		return -1;
	}

	newproc->size = curproc->size;
	newproc->parent = curproc;
	memmove(newproc->regs, curproc->regs, sizeof(*curproc->regs));
	newproc->regs->rax = 0;

	for (i = 0; i < NOFILE; i++)
		if (curproc->ofile[i])
			newproc->ofile[i] = filedup(curproc->ofile[i]);
	newproc->cwd = idup(curproc->cwd);		

	pid = newproc->pid;

	acquire(&ptable.lock);
	newproc->state = RUNNABLE;
	release(&ptable.lock);
	
	return pid;	
}

void exit(void){

	struct proc *curproc = myproc();

	if (curproc == initproc)
		panic("init exiting");

	acquire(&ptable.lock);	

	wakeup1(curproc->parent);

	curproc->state = ZOMBIE;
	sched();
}

int wait(void){

	struct proc *p, *curproc = myproc();
	int havechild, pid;

	acquire(&ptable.lock);
	for (;;){
		havechild = 0;
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++){
			if (p->parent != curproc) continue;
			havechild = 1;
			if (p->state == ZOMBIE){
				pid = p->pid;
				kfree(p->kstack);
				p->kstack = 0;
				freevm(p->pml4t);
				p->pid = 0;
				p->parent = 0;
				p->killed = 0;
				p->state = UNUSED;
				release(&ptable.lock);

				return pid;
			}
		}
		if (!havechild || curproc->killed){
			release(&ptable.lock);
			return -1;
		}
		sleep(curproc, &ptable.lock);
	}
}

void sched(void){

	int intena;
	struct proc *p = myproc();

	if (!holding(&ptable.lock))
		panic("sched");
	if (mycpu()->ncli != 1)
		panic("sched");
	if (readrflags()&FL_IF)
		panic("sched");		
	intena = mycpu()->intena;
	swtch(&p->context, &mycpu()->context);
	mycpu()->intena = intena;
}

void scheduler(void){

	struct cpu *c = mycpu();
	struct proc *p;

	for (;;){
		sti();

		acquire(&ptable.lock);
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++){
			if (p->state != RUNNABLE) continue;

			asm volatile("swapgs");

			c->proc = p;
			switchuvm(p);
			p->state = RUNNING;

			swtch(&c->context, &p->context);
			switchkvm();

			c->proc = 0;
		}
		release(&ptable.lock);
	}
}

void sleep(void *chan, struct spinlock *lock){

	struct proc *p = myproc();

	if (p == 0)
		panic("sleep");

	if (lock == 0)
		panic("sleep");	

	if (lock != &ptable.lock){
		acquire(&ptable.lock);
		release(lock);
	}

	p->chan = chan;
	p->state = SLEEPING;

	sched();

	p->chan = 0;

	if (lock != &ptable.lock){
		release(&ptable.lock);
		acquire(lock);
	}
}

void wakeup1(void *chan){

	struct proc *p;

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++){
		if (p->state == SLEEPING && p->chan == chan)
			p->state = RUNNABLE;
	}
}

void wakeup(void *chan){

	acquire(&ptable.lock);
	wakeup1(chan);
	release(&ptable.lock);
}

void yield(void){
	acquire(&ptable.lock);
	myproc()->state = RUNNABLE;
	sched();
	release(&ptable.lock);
}

void forkret(void){

	static int first = 1;

	release(&ptable.lock);

	if (first){
		first = 0;
		iinit(ROOTDEV);
	}
}
