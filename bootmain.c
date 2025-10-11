
#include "types.h"
#include "elf.h"
#include "mmu.h"
#include "x86.h"

#define SECTSIZE 512

void readseg(uint8_t*, uint32_t, uint32_t);

void bootmain(void){
	uint16_t *crt = (uint16_t*)0xb8000;

	struct elfhdr *elf;
	struct proghdr *ph, *eph;
	uint8_t *pa;
	void (*entry)(void);

	elf = (struct elfhdr*)0x10000;
	readseg((uint8_t*)elf, 0x1000, 0);

	if (elf->magic != ELF_MAGIC){
		crt[0] = 'E' | 0x0700;
		return;
	}

	ph = (struct proghdr*)((char*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for ( ;ph < eph; ph++){
		pa = (uint8_t*)ph->paddr;
		readseg(pa, ph->filesz, ph->offset);
		if (ph->memsz > ph->filesz)
			stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
	}
	entry = (void(*)(void))(elf->entry);
	entry();
}

void waitdisk(void){
	while ((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *addr, uint32_t offset){

	outb(0x1f2, 1);
	outb(0x1f3, offset);
	outb(0x1f7, 0x20);

	waitdisk();
	insl(0x1f0, addr, SECTSIZE/4);
}

void readseg(uint8_t *pa, uint32_t size, uint32_t offset){

	uint8_t *epa;

	epa = pa + size;
	offset = (offset / SECTSIZE) + 35;
	for (; pa < epa; pa += SECTSIZE, offset++)
		readsect(pa, offset);
}
