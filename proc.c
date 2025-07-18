
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

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == SLEEPING && p->chan == chan)
			p->state = RUNNABLE;
}

void wakeup(void *chan){
	acquire(&ptable.lock);
	wakeup1(chan);
	release(&ptable.lock);
}
