
#include "types.h"
#include "elf.h"
#include "x86.h"

#define SECTSIZE 512

void readsegment(uint8_t *, uint32_t, uint32_t);

void bootmain(void){
	uint16_t *crt = (void*)0xb8000;
	crt[0] = 0x0f00 | 'B';

	struct elfhdr *elf = (void*)0x10000;
	readsegment((void*)elf, 0, sizeof(*elf));
	if (elf->magic != ELF_MAGIC){
		crt[0] = 0x0f00 | 'A';
		return;
	}

	struct proghdr *ph, *eph;
	ph = (void*)((uint8_t*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for (; ph < eph; ph++){
		readsegment((void*)ph->paddr, ph->offset, ph->filesz);
		if (ph->memsz > ph->filesz)
			stosb((uint8_t*)ph->paddr+ph->filesz, 0, ph->memsz-ph->filesz);
	}
	void (*entry)(void) = (void*)(elf->entry);
	entry();
}

void waitdisk(void){
	while ((inb(0x1f7) & 0xc0) != 0x40);
}

void readsector(void *addr, uint32_t sector){

	outb(0x1f2, 1);
	outb(0x1f3, sector);
	outb(0x1f4, sector >> 8);
	outb(0x1f5, sector >> 16);
	outb(0x1f6, 0xe0 | sector >> 24);
	outb(0x1f7, 0x20);

	waitdisk();
	insl(0x1f0, addr, SECTSIZE/4);
}

void readsegment(uint8_t *pa, uint32_t offset, uint32_t size){

	uint8_t *epa = pa + size;
	uint64_t sector = 27 + offset/SECTSIZE;

	for (; pa < epa; pa+=SECTSIZE, sector++)
		readsector(pa, sector);
}
