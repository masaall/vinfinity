
#include "memlayout.h"

.globl _start
_start = V2P_WO(entry)
entry:

	movq $V2P_WO(entrypml4t),%rax
	movq %rax,%cr3

	movq $(stack + 4096),%rsp

	movq $main,%rax
	jmp *%rax

.comm stack, 4096
