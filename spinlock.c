
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "x86.h"

void acquire(struct spinlock *lock){

	pushcli();
	if (holding(lock))
		panic("acquire");

	while (xchg((uintptr_t*)&lock->locked, 1) != 0);

	__sync_synchronize();

	lock->cpu = mycpu();
}

void release(struct spinlock *lock){

	if (!holding(lock))
		panic("release");

	lock->cpu = 0;

	__sync_synchronize();

	asm volatile("movq $0, %0" : "+m" (lock->locked) : );

	popcli();	
}

void getcallerpcs(void *v, uintptr_t pcs[]){

	uintptr_t *rbp;
	int32_t i;

	rbp = (uintptr_t*)v;
	cprintf("rbp %p rbp[0] %p rbp[1] %p\n", rbp, rbp[0], rbp[1]);
	for (i = 0; i < 10; i++){
		if (rbp == 0 || rbp < (uintptr_t*)KERNBASE || rbp == (uintptr_t*)0xffffffffffffffff)
			break;
		pcs[i] = rbp[1];
		rbp = (uintptr_t*)rbp[0];	
	}
	for (; i < 10; i++)
		pcs[i] = 0;
}

int32_t holding(struct spinlock *lock){
	int32_t r;
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
		panic("popcli - interruptible");
	if (--mycpu()->ncli < 0)
		panic("popcli");
	if (mycpu()->ncli == 0 && mycpu()->intena)
		sti();		
}
