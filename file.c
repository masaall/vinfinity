
#include "types.h"
#include "defs.h"
#include "param.h"
#include "gdt.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

struct devsw devsw[NDEV];

struct {
	struct spinlock lock;
	struct file file[NFILE];
} ftable;

void fileinit(void){
	initlock(&ftable.lock, "ftable");
}

struct file *filealloc(void){

	struct file *f;

	acquire(&ftable.lock);
	for (f = ftable.file; f < &ftable.file[NFILE]; f++){
		if (f->ref == 0){
			f->ref = 1;
			release(&ftable.lock);
			return f;
		}
	}
	release(&ftable.lock);

	return 0;
}

struct file *filedup(struct file *f){

	acquire(&ftable.lock);
	if (f->ref < 1) panic("filedup");
	f->ref++;
	release(&ftable.lock);

	return f;
}

void fileclose(struct file *f){
	acquire(&ftable.lock);
	if (f->ref < 1) panic("fileclose");
	if (--f->ref > 0){
		release(&ftable.lock);
		return;
	}

	release(&ftable.lock);

}

int filewrite(struct file *f, char *src, int n){

	if (f->writable == 0)
		return -1;

	if (f->type == FD_INODE){
		ilock(f->ip);
		if ((n = writei(f->ip, src, f->off, n)) < 0)
			panic("filewrite");
		iunlock(f->ip);
			
		return n;
	}	
	panic("filewrite");
}
