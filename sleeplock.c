
#include "types.h"
#include "defs.h"
#include "gdt.h"
#include "list.h"
#include "proc.h"
#include "sleeplock.h"

void acquiresleep(struct sleeplock *lock){

	if (lock->locked){
		sleep(lock);
	}

	lock->locked = true;
	lock->pid = myproc()->pid;
}

void releasesleep(struct sleeplock *lock){

	lock->locked = false;
	lock->pid = 0;
	wakeup(lock);
}

bool holdingsleep(struct sleeplock *lock){
	return lock->locked && lock->pid == myproc()->pid;
}
