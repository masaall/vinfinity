
#include "types.h"
#include "elf.h"
#include "mmu.h"
#include "x86.h"

#define SECTSIZE 512

void readseg(char*, uintptr_t, uintptr_t);

void bootmain(void){
	uint16_t *crt = (void*)0xb8000;

	struct elfhdr *elf;
	struct proghdr *ph, *eph;
	void (*entry)(void);

	elf = (struct elfhdr*)0x10000;
	readseg((void*)elf, sizeof(struct elfhdr), 0);

	if (elf->magic != ELF_MAGIC){
		crt[0] = 'E' | 0x0700;
		return;
	}

	ph = (struct proghdr*)((char*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for (; ph < eph; ph++){
		readseg((void*)ph->paddr, ph->filesz, ph->offset);
		if (ph->memsz > ph->filesz)
			stosb((void*)ph->paddr+ph->filesz, 0, ph->memsz-ph->filesz);	
	}
	entry = (void*)(elf->entry);
	entry(); 
}

void waitdisk(void){
	while ((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *addr, uintptr_t sector){

	outb(0x1f2, 1);
	outb(0x1f3, sector);
	outb(0x1f4, sector >> 8);
	outb(0x1f5, sector >> 16);
	outb(0x1f6, 0xe0 | (sector >> 24));
	outb(0x1f7, 0x20);

	waitdisk();
	insl(0x1f0, addr, SECTSIZE/4);
}

void readseg(char *paddr, uintptr_t size, uintptr_t offset){

	char *epaddr;
	uintptr_t sector;

	epaddr = paddr + size;
	sector = (offset / SECTSIZE) + 35;
	for (; paddr < epaddr; paddr += SECTSIZE, sector++)
		readsect(paddr, sector);
}
