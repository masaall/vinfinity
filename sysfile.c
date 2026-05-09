
#include "types.h"
#include "defs.h"
#include "param.h"
#include "idt.h"

int sys_write(struct registers *regs){

	const char *s;
	size_t n;

	s = (char*)regs->rdi;
	n = regs->rsi;

	return consolewrite1(s, n);
}

int sys_exec(struct registers *regs){

	char *path, *argv[MAXARG];
	uintptr_t uargv, uarg;

	path = (char*)regs->rdi;
	uargv = regs->rsi;

	for (int i = 0; i < MAXARG; i++){
		uarg = *((uintptr_t*)uargv+i);
		
		if (uarg == 0){
			argv[i] = 0;
			break;
		}
		argv[i] = (char*)uarg;
	}

	return exec(path, argv);
}
