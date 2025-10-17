
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "fs.h"
#include "file.h"
#include "elf.h"
#include "irqs.h"
#include "x86.h"

int exec(char *path, char **argv){

	struct inode *ip;
	struct elfhdr elf;
	struct proghdr ph;
	struct proc *curproc = myproc();
	uintptr_t *pml4t, *oldpml4t;
	uint32_t i, off;
	uintptr_t size;

	if ((ip = namei(path)) == 0){
		cprintf("exec: fail\n");
		return -1;
	}
	ilock(ip);
	pml4t = 0;

	if (readi(ip, (void*)&elf, 0, sizeof(elf)) != sizeof(elf))
		goto bad;
	if (elf.magic != ELF_MAGIC)
		goto bad;

	pml4t = setupkvm();

	size = 0;
	for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)){
		if (readi(ip, (void*)&ph, off, sizeof(ph)) != sizeof(ph))
			goto bad;
		size = allocuvm(pml4t, size, ph.vaddr+ph.memsz);
		if (loaduvm(pml4t, ip, ph.vaddr, ph.offset, ph.filesz) < 0)
			goto bad;	
	}
	iunlockput(ip);
	ip = 0;

	size = PGUP(size);
	size = allocuvm(pml4t, size, size + 2*PGSIZE);

	oldpml4t = curproc->pml4t;
	curproc->pml4t = pml4t;
	curproc->size = size;
	curproc->regs->rip = elf.entry;
	switchuvm(curproc);
	freevm(oldpml4t);

	return 0;
bad:
	if (pml4t) 
		freevm(pml4t);
		
	return -1;
}
