
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "x86.h"
#include "spinlock.h"

struct {
	struct spinlock lock;
	struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

void pinit(void){
	initlock(&ptable.lock, "ptable");
}

struct cpu *mycpu(void){

	int32_t apicid, i;

	if (readrflags()&FL_IF)
		panic("mycpu called with interrupts enabled\n");

	apicid = lapicid();
	for (i = 0; i < ncpu; i++)
		if (cpus[i].apicid == apicid)
			return &cpus[i];	

	panic("unknown apicid\n");
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
	char *sp;

	acquire(&ptable.lock);

	for (p = ptable.proc; p < ptable.proc + NPROC; p++)
		if (p->state == UNUSED)
			goto found;

	release(&ptable.lock);
	return 0;		

found:
	p->state = EMBRYO;
	p->pid = nextpid++;

	release(&ptable.lock);

	if ((p->kstack = kalloc()) == 0){
		p->state = UNUSED;
		return 0;
	}

	sp = p->kstack + KSTACKSIZE;

	sp -= sizeof *p->tf;
	p->tf = (struct trapframe*)sp;

	sp -= 8;
	*(uintptr_t*)sp = (uintptr_t)trapret;

	sp -= sizeof *p->context;
	p->context = (struct context*)sp;
	memset(p->context, 0, sizeof *p->context);
	p->context->rip = (uintptr_t)forkret;

	return p;		
}

void userinit(void){

	struct proc *p;
	extern char _binary_initcode_start[], _binary_initcode_size[];

	p = allocproc();
	
	initproc = p;
	if ((p->pml4 = setupkvm()) == 0)
		panic("userinit: ");
	inituvm(p->pml4, _binary_initcode_start, (uintptr_t)_binary_initcode_size);	
	p->size = PGSIZE;
	memset(p->tf, 0, sizeof(*p->tf));	
	p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
	p->tf->ss = (SEG_UDATA << 3) | DPL_USER;
	p->tf->rflags = FL_IF;
	p->tf->rsp = PGSIZE;
	p->tf->rip = 0;

	acquire(&ptable.lock);	
	p->state = RUNNABLE;
	release(&ptable.lock);
}

void exit(void){

	struct proc *curproc = myproc();
	int fd;

	if (curproc == initproc)
		panic("init exiting");

	for (fd = 0; fd < NOFILE; fd++){
		
	}	
}

void scheduler(void){

	struct proc *p;
	struct cpu *c = mycpu();
	c->proc = 0;

	for (;;){	
		sti();
		acquire(&ptable.lock);
		for (p = ptable.proc; p < ptable.proc + NPROC; p++){
			if (p->state != RUNNABLE)
				continue;

			c->proc = p;
			switchuvm(p);	
			p->state = RUNNING;

			swtch(&c->scheduler, p->context);
			switchkvm();

			c->proc = 0;	
		}
		release(&ptable.lock); 
	}
}

void sched(void){

	struct proc *p = myproc();

	if (!holding(&ptable.lock))
		panic("sched ptable lock");
	if (mycpu()->ncli != 1)
		panic("sched locks");
	if (p->state == RUNNING)
		panic("sched running");
	if (readrflags()&FL_IF)
		panic("sched interruptible");			
	swtch(&p->context, mycpu()->scheduler);
}

void forkret(void){

	static int32_t first = 1;

	release(&ptable.lock);

	if (first){
		first = 0;
	}
}

void sleep(void *chan, struct spinlock *lock){

	struct proc *p = myproc();

	if (p == 0)
		panic("sleep");

	if (lock == 0)
		panic("sleep without lock");

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

	for (p = ptable.proc; p < ptable.proc + NPROC; p++)
		if (p->state == SLEEPING && p->chan == chan)
			p->state = RUNNABLE;
}

void wakeup(void *chan){
	acquire(&ptable.lock);
	wakeup1(chan);
	release(&ptable.lock);
}
