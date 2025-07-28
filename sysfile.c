
#include "types.h"
#include "defs.h"
#include "param.h"
#include "proc.h"
#include "x86.h"

int sys_write(void){

	return 0;
}

int sys_cwrite(void){

	char *addr;
	int n;
	struct proc *p = myproc();

	addr = (char*)p->tf->rdi;
	n = p->tf->rsi;

	return consolewrite1(addr, n);
}

int sys_exec(void){

	struct proc *p = myproc();
	char *path, *argv[MAXARG];
	uintptr_t uargv, uarg;
	int i;

	path = (char*)p->tf->rdi;
	uargv = p->tf->rsi;

	memset(argv, 0, sizeof(argv));
	for (i=0;;i++){
		uarg = *(char*)(uargv+8*i);	
		if (uarg == 0){
			argv[i] = 0;
			break;
		}	
		argv[i] = (char*)uarg;
	}
	return exec(path, argv);
}
