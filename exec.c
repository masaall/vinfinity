
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "gdt.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "elf.h"
#include "irqs.h"
#include "x86.h"

#define SECTSIZE 512

int exec(char *path, char **argv){

	struct inode *ip;
	struct elfhdr elf;
	struct proghdr ph;
	struct proc *curproc = myproc();
	uintptr_t *pml5t, *oldpml5t;
	uint32_t i, off, argc;
	uintptr_t size, stack;

	if ((ip = namei(path)) == 0){
		cprintf("exec: fail\n");
		return -1;
	}

	ilock(ip);
	pml5t = 0;

	if (readi(ip, &elf, 0, sizeof(elf)) != sizeof(elf))
		goto bad;
	if (elf.magic != ELF_MAGIC)
		goto bad;	

	if ((pml5t = setupkvm()) == 0)
		goto bad;

	size = 0;
	for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)){
		if (readi(ip, &ph, off, sizeof(ph)) != sizeof(ph))
			goto bad;		
		if ((size = allocuvm(pml5t, size, ph.vaddr+ph.memsz)) == 0)
			goto bad;	
		if (loaduvm(pml5t, ip, ph.vaddr, ph.offset, ph.filesz) < 0)
			goto bad;
	}
	
	iunlockput(ip);
	ip = 0;

	size = PGUP(size);
	if ((size = allocuvm(pml5t, size, size + 2*PGSIZE)) == 0)
		goto bad;
	stack = size;	

	for (argc = 0; argv[argc]; argc++){
		
	}

	oldpml5t = curproc->pml5t;
	curproc->pml5t = pml5t;
	curproc->size = size;
	curproc->regs->rip = elf.entry;
	curproc->regs->rsp = stack;
	switchuvm(curproc);
	freevm(oldpml5t);

	return 0;
bad:
	if (pml5t) 
		freevm(pml5t);
	if (ip){
		iunlockput(ip);
	}
	return -1;
}
