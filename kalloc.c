
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"

extern char end[];

void freerange(char*, char*);

struct free {
	struct free *next;
};

struct free *freep;

void kinit1(void *start, void *end){
	freerange(start, end);
}

void kinit2(void *start, void *end){
	freerange(start, end);
}

void freerange(char *start, char *end){
	start = (char*)PGUP((uintptr_t)start);
	for (; start < end; start += PGSIZE)
		kfree(start);
}

void kfree(char *addr){

	struct free *free;

	if ((uintptr_t)addr % PGSIZE || addr < end || V2P(addr) >= PHYSTOP)
		panic("kfree");

	free = (struct free*)addr;
	free->next = freep;
	freep = free;
}

void *kalloc(void){

	struct free *free;

	free = freep;
	if (free){
		freep = free->next;
		memset(free, 0, PGSIZE);
	} 
	
	return free;
}

void getcallerpcs(uintptr_t *rbp, uintptr_t *pcs){

	uint8_t i;

	for (i = 0; i < 10; i++){
		if (rbp == 0 || rbp < (uintptr_t*)KERNBASE)
			break;	
		pcs[i] = rbp[1];
		rbp = (uintptr_t*)rbp[0];
	}
	for (; i < 10; i++)
		pcs[i] = 0;
}
