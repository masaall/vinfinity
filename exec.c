
#include "types.h"
#include "defs.h"
#include "sleeplock.h"
#include "gdt.h"
#include "list.h"
#include "idt.h"
#include "proc.h"
#include "mmu.h"
#include "fs.h"
#include "file.h"
#include "elf.h"

int exec(char *path, char *argv[]){

	struct inode *ip;
	struct elfhdr elf;
	struct proghdr ph;
	int i, off;
	uintptr_t size, *pml5t, *oldpml5t, sp;

	if ((ip = namei(path)) == 0){
		cprintf("exec failed!\n");
		return -1;
	}

	pml5t = setupkvm();

	ilock(ip);

	readi(ip, (char*)&elf, 0, sizeof(elf));

	size = 0;
	for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)){
		readi(ip, (char*)&ph, off, sizeof(ph));

		size = allocuvm(pml5t, size, ph.vaddr+ph.memsz);

		loaduvm(ip, pml5t, ph.vaddr, ph.offset, ph.filesz);
	}
	iunlock(ip);

	size = PGUP(size);
	size = allocuvm(pml5t, size, size+2*PGSIZE);

	sp = size;

	oldpml5t = myproc()->pml5t;
	myproc()->pml5t = pml5t;
	myproc()->size = size;
	myproc()->regs->rip = elf.entry;
	myproc()->regs->rsp = sp;
	switchuvm(myproc());
	freevm(oldpml5t, myproc()->size);

	return 0;
}
