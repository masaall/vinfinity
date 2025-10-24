
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"

void initsleeplock(struct sleeplock *lock, char *name){
	initlock(&lock->lock, "sleeplock");
	lock->name = name;
	lock->locked = 0;
	lock->pid = 0;
}

void acquiresleep(struct sleeplock *lock){

	acquire(&lock->lock);
	while (lock->locked){
		sleep(lock, &lock->lock);
	}
	lock->locked = 1;
	lock->pid = myproc()->pid;
	release(&lock->lock);
}

void releasesleep(struct sleeplock *lock){

	acquire(&lock->lock);
	lock->locked = 0;
	lock->pid = 0;
	wakeup(lock);
	release(&lock->lock);
}

int holdingsleep(struct sleeplock *lock){

	int r;

	acquire(&lock->lock);
	r = lock->locked && lock->pid == myproc()->pid;
	release(&lock->lock);

	return r;
}
