
#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "file.h"

struct devsw devsw[NDEV];

struct {
	struct file file[NFILE];
} ftable;

struct file *filealloc(void){

	struct file *f;

	for (f = ftable.file; f < &ftable.file[NFILE]; f++){
		if (f->ref == 0){
			f->ref = 1;
			return f;
		}
	}

	return 0;
}

struct file *filedup(struct file *f){
	if (f->ref < 1) panic("filedup");
	f->ref++;

	return f;
}

void fileclose(struct file *f){
	if (f->ref < 1) panic("fileclose");
	if (--f->ref > 0)
		return;
}

int filewrite(struct file *f, char *src, int n){

	if (f->writable == 0)
		return -1;

	if (f->type == FD_INODE){
		if ((n = writei(f->ip, src, f->off, n)) < 0)
			panic("filewrite");

		return n;
	}	
	panic("filewrite");
}
