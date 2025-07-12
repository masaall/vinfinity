
#include "types.h"
#include "elf.h"
#include "x86.h"

#define SECTSIZE 512

void readseg(uint8_t*, uint64_t, uint64_t);

void bootmain(void){
	char *video = (char*)0xb8000;
	*video = 'A';

	struct elf64_header *elf;
	struct elf64_phdr *ph, *eph;
	uint8_t *pa;
	void (*entry)(void);

	elf = (struct elf64_header*)0x10000;

	readseg((uint8_t*)elf, 0x1000, 0);

	if (elf->e_magic != ELF_MAGIC){
		*video = 'E';
		return;
	}

	ph = (struct elf64_phdr*)((uint8_t*)elf + elf->e_phoff);
	eph = ph + elf->e_phnum;
	for (; ph < eph; ph++){
		pa = (uint8_t*)ph->p_paddr;
		readseg(pa, ph->p_filesz, ph->p_offset);
		if (ph->p_memsz > ph->p_filesz)
			stosb(pa + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
	}
	entry = (void(*)(void))(elf->e_entry);
	entry();
}

void waitdisk(void){
	while((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *dst, uint64_t offset){

	waitdisk();
	outb(0x1f2, 1);
	outb(0x1f3, offset);
	outb(0x1f4, offset >> 8);
	outb(0x1f5, offset >> 16);
	outb(0x1f6, (offset >> 24) | 0xe0);
	outb(0x1f7, 0x20);

	waitdisk();
	insl(0x1f0, dst, SECTSIZE/4);
}

void readseg(uint8_t *pa, uint64_t count, uint64_t offset){

	uint8_t *epa;

	epa = pa + count;

	pa -= offset % SECTSIZE;

	offset = (offset / SECTSIZE) + 35;

	for (; pa < epa; pa += SECTSIZE, offset++)
		readsect(pa, offset);
}
