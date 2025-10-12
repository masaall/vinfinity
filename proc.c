
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "irqs.h"
#include "x86.h"

extern void touser(void);
extern void touser1(void);

struct {
	struct proc proc[NPROC];
} ptable;

struct proc *initproc;
int nextpid = 1;

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

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == UNUSED)
			goto found;

	return 0;	
found:
	p->pid = nextpid++;
	p->state = EMBRYO;

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
		*(uintptr_t*)stack = (uintptr_t)touser;
	} else {
		*(uintptr_t*)stack = (uintptr_t)touser1;
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

	p->regs->cs = 0x2b;
	p->regs->ss = 0x23;
	p->regs->rsp = PGSIZE;
	p->regs->rflags = FL_IF;
	p->regs->rip = 0;

	p->cwd = namei("/");
	p->state = RUNNABLE;
}

int growproc(int n){

	struct proc *curproc = myproc();
	uintptr_t size;

	size = curproc->size;
	if (n > 0){
		size = allocuvm(curproc->pml4t, size, size + n);
	} else if (n < 0) {
		size = deallocuvm(curproc->pml4t, size, size + n);
	}
	curproc->size = size;
	
	return 0;
}

int fork(void){

	struct proc *newproc, *curproc = myproc();
	int pid, i;

	if ((newproc = allocproc()) == 0)
		return -1;

	newproc->pml4t = copyuvm(curproc->pml4t, curproc->size);

	newproc->size = curproc->size;
	newproc->parent = curproc;
	memmove(newproc->regs, curproc->regs, sizeof(*curproc->regs));
	newproc->regs->rax = 0;

	for (i = 0; i < NOFILE; i++)
		if (curproc->ofile[i])
			newproc->ofile[i] = filedup(curproc->ofile[i]);
	newproc->cwd = idup(curproc->cwd);		

	pid = newproc->pid;
	newproc->state = RUNNABLE;
	
	return pid;
}

void exit(void){

	struct proc *curproc = myproc();

	if (curproc == initproc)
		panic("init exiting");

	wakeup(curproc->parent);

	curproc->state = ZOMBIE;
	sched();
}

int wait(void){

	struct proc *p, *curproc = myproc();
	int havechild, pid;

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

				return pid;
			}
		}
		if (!havechild){
			return -1;
		}
		sleep(curproc);
	}
}

void sched(void){

	struct cpu *c = mycpu();
	struct proc *p = myproc();

	swtch(&p->context, c->context);
}

void scheduler(void){

	struct cpu *c = mycpu();
	struct proc *p;

	for (;;){
		sti();
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++){
			if (p->state != RUNNABLE) continue;

			c->proc = p;
			p->state = RUNNING;
			switchuvm(p);
			swtch(&c->context, p->context);
			switchkvm();

			c->proc = 0;
		}
	}
}

void sleep(void *chan){

	struct proc *p = myproc();

	pushcli();

	p->state = SLEEPING;
	p->chan = chan;

	sched();

	p->chan = 0;

	popcli();

}

void wakeup(void *chan){

	struct proc *p;

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++){
		if (p->state == SLEEPING && p->chan == chan)
			p->state = RUNNABLE;
	}
}

void forkret(void){

	static bool first = true;

	if (first){
		first = false;
		iinit(ROOTDEV);
	}
}
