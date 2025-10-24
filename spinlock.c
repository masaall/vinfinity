
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "spinlock.h"
#include "x86.h"

void initlock(struct spinlock *lock, char *name){
	lock->name = name;
	lock->locked = 0;
	lock->cpu = 0;
}

void acquire(struct spinlock *lock){

	pushcli();
	if (holding(lock)) panic("acquire");

	while (xchg((uintptr_t*)&lock->locked, 1) != 0);

	__sync_synchronize();

	lock->cpu = mycpu();
}

void release(struct spinlock *lock){

	if (!holding(lock))
		panic("release");

	lock->cpu = 0;

	__sync_synchronize();

	asm volatile("mov $0,%0" : "+m" (lock->locked));

	popcli();
}

int holding(struct spinlock *lock){

	int r;
	pushcli();
	r = lock->locked && lock->cpu == mycpu();
	popcli();

	return r;
}

void pushcli(void){

	int eflags;

	eflags = readrflags();
	cli();
	if (mycpu()->ncli == 0)
		mycpu()->intena = eflags & FL_IF;
	mycpu()->ncli += 1;
}

void popcli(void){

	if (readrflags()&FL_IF)
		panic("popcli 1");
	if (--mycpu()->ncli < 0)
		panic("popcli 2");
	if (mycpu()->ncli == 0 && mycpu()->intena)
		sti();		
}
