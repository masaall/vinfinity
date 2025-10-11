
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "proc.h"
#include "fs.h"
#include "file.h"
#include "irqs.h"
#include "fcntl.h"
#include "stat.h"

int fdalloc(struct file *f){

	int fd;
	struct proc *p = myproc();

	for (fd = 0; fd < NOFILE; fd++){
		if (p->ofile[fd] == 0){
			p->ofile[fd] = f;
			return fd;
		}
	}

	return -1;
}

int sys_dup(void){

	struct proc *p = myproc();
	struct file *f;
	int fd;

	fd = p->regs->rdi;

	if (fd < 0 || fd >= NOFILE || (f = p->ofile[fd]) == 0)
		return -1;

	if ((fd = fdalloc(f)) < 0)
		return -1;
	filedup(f);		

	return fd;
}

int sys_write(void){

	struct proc *p = myproc();
	struct file *f;
	int fd;
	char *addr;
	int n;

	fd = p->regs->rdi;
	addr = (char*)p->regs->rsi;
	n = p->regs->rdx;

	if (fd < 0 || fd >= NOFILE || (f = p->ofile[fd]) == 0)
		return -1;

	return filewrite(f, addr, n);
}

struct inode *create(char *path, short type, short major, short minor){

	struct inode *dp, *ip;
	char name[DIRSIZ];

	if ((dp = nameiparent(path, name)) == 0)
		return 0;

	ilock(dp);	

	ip = ialloc(dp->dev, type);

	ilock(ip);
	ip->major = major;
	ip->minor = minor;
	ip->nlink = 1;
	iupdate(ip);

	if (dirlink(dp, name, ip->inum) < 0)
		panic("create");

	iunlockput(dp);	

	return ip;
}

int sys_open(void){

	struct proc *p = myproc();
	struct inode *ip;
	struct file *f;
	char *path;
	int omode, fd;

	path = (char*)p->regs->rdi;
	omode = p->regs->rsi;

	if ((ip = namei(path)) == 0){
			return -1;
	}

	if ((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
		if (f) fileclose(f);
		return -1;
	}

	f->type = FD_INODE;
	f->ip = ip;
	f->off = 0;
	f->readable = (omode & O_RDONLY) || (omode & O_RDWR);
	f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
	
	return fd;
}

int sys_mknod(void){

	struct proc *p = myproc();
	struct inode *ip;
	char *path;
	short major, minor;

	path = (char*)p->regs->rdi;
	major = p->regs->rsi;
	minor = p->regs->rdx;

	if ((ip = create(path, T_DEV, major, minor)) == 0){
		return -1;
	}
	
	return 0;
}

int sys_exec(void){

	struct proc *p = myproc();
	char *path, *argv[MAXARG];
	uintptr_t uargv, uarg;
	uint8_t i;

	path = (char*)p->regs->rdi;
	uargv = p->regs->rsi;

	for (i = 0; i < 10; i++){
		uarg = *((uintptr_t*)uargv+i);
		if (uarg == 0) break;
		argv[i] = (char*)uarg;
	}
	return exec(path, argv);
}
